#pragma once

/** Inode number type */
typedef uint64_t fuse_ino_t;

/** Request pointer type */
typedef struct fuse_req *fuse_req_t;

typedef ssize_t off_t;

/**
 * Session
 *
 * This provides hooks for processing requests, and exiting
 */
struct fuse_session;

struct fuse_pollhandle;
struct fuse_conn_info_opts;

/**
 * Argument list
 */
struct fuse_args {
  /** Argument count */
  int argc;

  /** Argument vector.  NULL terminated */
  char **argv;

  /** Is 'argv' allocated? */
  int allocated;
};

/**
 * Connection information, passed to the ->init() method
 *
 * Some of the elements are read-write, these can be changed to
 * indicate the value requested by the filesystem.  The requested
 * value must usually be smaller than the indicated value.
 */
struct fuse_conn_info {
  /**
   * Major version of the protocol (read-only)
   */
  unsigned proto_major;

  /**
   * Minor version of the protocol (read-only)
   */
  unsigned proto_minor;

  /**
   * Maximum size of the write buffer
   */
  unsigned max_write;

  /**
   * Maximum size of read requests. A value of zero indicates no
   * limit. However, even if the filesystem does not specify a
   * limit, the maximum size of read requests will still be
   * limited by the kernel.
   *
   * NOTE: For the time being, the maximum size of read requests
   * must be set both here *and* passed to fuse_session_new()
   * using the ``-o max_read=<n>`` mount option. At some point
   * in the future, specifying the mount option will no longer
   * be necessary.
   */
  unsigned max_read;

  /**
   * Maximum readahead
   */
  unsigned max_readahead;

  /**
   * Capability flags that the kernel supports (read-only)
   */
  unsigned capable;

  /**
   * Capability flags that the filesystem wants to enable.
   *
   * libfuse attempts to initialize this field with
   * reasonable default values before calling the init() handler.
   */
  unsigned want;

  /**
   * Maximum number of pending "background" requests. A
   * background request is any type of request for which the
   * total number is not limited by other means. As of kernel
   * 4.8, only two types of requests fall into this category:
   *
   *   1. Read-ahead requests
   *   2. Asynchronous direct I/O requests
   *
   * Read-ahead requests are generated (if max_readahead is
   * non-zero) by the kernel to preemptively fill its caches
   * when it anticipates that userspace will soon read more
   * data.
   *
   * Asynchronous direct I/O requests are generated if
   * FUSE_CAP_ASYNC_DIO is enabled and userspace submits a large
   * direct I/O request. In this case the kernel will internally
   * split it up into multiple smaller requests and submit them
   * to the filesystem concurrently.
   *
   * Note that the following requests are *not* background
   * requests: writeback requests (limited by the kernel's
   * flusher algorithm), regular (i.e., synchronous and
   * buffered) userspace read/write requests (limited to one per
   * thread), asynchronous read requests (Linux's io_submit(2)
   * call actually blocks, so these are also limited to one per
   * thread).
   */
  unsigned max_background;

  /**
   * Kernel congestion threshold parameter. If the number of pending
   * background requests exceeds this number, the FUSE kernel module will
   * mark the filesystem as "congested". This instructs the kernel to
   * expect that queued requests will take some time to complete, and to
   * adjust its algorithms accordingly (e.g. by putting a waiting thread
   * to sleep instead of using a busy-loop).
   */
  unsigned congestion_threshold;

  /**
   * When FUSE_CAP_WRITEBACK_CACHE is enabled, the kernel is responsible
   * for updating mtime and ctime when write requests are received. The
   * updated values are passed to the filesystem with setattr() requests.
   * However, if the filesystem does not support the full resolution of
   * the kernel timestamps (nanoseconds), the mtime and ctime values used
   * by kernel and filesystem will differ (and result in an apparent
   * change of times after a cache flush).
   *
   * To prevent this problem, this variable can be used to inform the
   * kernel about the timestamp granularity supported by the file-system.
   * The value should be power of 10.  The default is 1, i.e. full
   * nano-second resolution. Filesystems supporting only second resolution
   * should set this to 1000000000.
   */
  unsigned time_gran;

  /**
   * For future use.
   */
  unsigned reserved[22];
};

/**
 * Information about an open file.
 *
 * File Handles are created by the open, opendir, and create methods and closed
 * by the release and releasedir methods.  Multiple file handles may be
 * concurrently open for the same file.  Generally, a client will create one
 * file handle per file descriptor, though in some cases multiple file
 * descriptors can share a single file handle.
 */
struct fuse_file_info {
  /** Open flags.	 Available in open() and release() */
  int flags;

  /** In case of a write operation indicates if this was caused
      by a delayed write from the page cache. If so, then the
      context's pid, uid, and gid fields will not be valid, and
      the *fh* value may not match the *fh* value that would
      have been sent with the corresponding individual write
      requests if write caching had been disabled. */
  unsigned int writepage : 1;

  /** Can be filled in by open, to use direct I/O on this file. */
  unsigned int direct_io : 1;

  /** Can be filled in by open. It signals the kernel that any
      currently cached file data (ie., data that the filesystem
      provided the last time the file was open) need not be
      invalidated. Has no effect when set in other contexts (in
      particular it does nothing when set by opendir()). */
  unsigned int keep_cache : 1;

  /** Indicates a flush operation.  Set in flush operation, also
      maybe set in highlevel lock operation and lowlevel release
      operation. */
  unsigned int flush : 1;

  /** Can be filled in by open, to indicate that the file is not
      seekable. */
  unsigned int nonseekable : 1;

  /* Indicates that flock locks for this file should be
     released.  If set, lock_owner shall contain a valid value.
     May only be set in ->release(). */
  unsigned int flock_release : 1;

  /** Can be filled in by opendir. It signals the kernel to
      enable caching of entries returned by readdir().  Has no
      effect when set in other contexts (in particular it does
      nothing when set by open()). */
  unsigned int cache_readdir : 1;

  /** Padding.  Reserved for future use*/
  unsigned int padding : 25;
  unsigned int padding2 : 32;

  /** File handle id.  May be filled in by filesystem in create,
   * open, and opendir().  Available in most other file operations on the
   * same file handle. */
  uint64_t fh;

  /** Lock owner id.  Available in locking operations and flush */
  uint64_t lock_owner;

  /** Requested poll events.  Available in ->poll.  Only set on kernels
      which support it.  If unsupported, this field is set to zero. */
  uint32_t poll_events;
};

struct cuse_info {
  unsigned dev_major;
  unsigned dev_minor;
  unsigned dev_info_argc;
  const char **dev_info_argv;
  unsigned flags;
};

/*
 * Most ops behave almost identically to the matching fuse_lowlevel
 * ops except that they don't take @ino.
 *
 * init_done	: called after initialization is complete
 * read/write	: always direct IO, simultaneous operations allowed
 * ioctl	: might be in unrestricted mode depending on ci->flags
 */
struct cuse_lowlevel_ops {
  void (*init)(void *userdata, struct fuse_conn_info *conn);
  void (*init_done)(void *userdata);
  void (*destroy)(void *userdata);
  void (*open)(fuse_req_t req, struct fuse_file_info *fi);
  void (*read)(fuse_req_t req, size_t size, off_t off,
               struct fuse_file_info *fi);
  void (*write)(fuse_req_t req, const char *buf, size_t size, off_t off,
                struct fuse_file_info *fi);
  void (*flush)(fuse_req_t req, struct fuse_file_info *fi);
  void (*release)(fuse_req_t req, struct fuse_file_info *fi);
  void (*fsync)(fuse_req_t req, int datasync, struct fuse_file_info *fi);
  void (*ioctl)(fuse_req_t req, int cmd, void *arg, struct fuse_file_info *fi,
                unsigned int flags, const void *in_buf, size_t in_bufsz,
                size_t out_bufsz);
  void (*poll)(fuse_req_t req, struct fuse_file_info *fi,
               struct fuse_pollhandle *ph);
};

struct fuse_session *cuse_lowlevel_new(struct fuse_args *args,
                                       const struct cuse_info *ci,
                                       const struct cuse_lowlevel_ops *clop,
                                       void *userdata);

struct fuse_session *cuse_lowlevel_setup(int argc, char *argv[],
                                         const struct cuse_info *ci,
                                         const struct cuse_lowlevel_ops *clop,
                                         int *multithreaded, void *userdata);

void cuse_lowlevel_teardown(struct fuse_session *se);

int cuse_lowlevel_main(int argc, char *argv[], const struct cuse_info *ci,
                       const struct cuse_lowlevel_ops *clop, void *userdata);
