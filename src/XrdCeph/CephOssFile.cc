#include <sys/types.h>
#include <unistd.h>

#include <XrdCeph/ceph_posix.h>
#include <XrdOuc/XrdOucEnv.hh>
#include <XrdSys/XrdSysError.hh>
#include <XrdOuc/XrdOucTrace.hh>
#include <XrdSfs/XrdSfsAio.hh>

#include "CephOssFile.hh"
#include "CephOss.hh"

extern XrdSysError CephEroute;

CephOssFile::CephOssFile(CephOss *cephOss) : m_fd(-1), m_cephOss(cephOss) {}

int CephOssFile::Open(const char *path, int flags, mode_t mode, XrdOucEnv &env) {
  try {
    m_fd = ceph_posix_open(&env, path, flags, mode);
    return XrdOssOK;
  } catch (std::exception e) {
    CephEroute.Say("open : invalid syntax in file parameters");
    return -EINVAL;
  }
}

int CephOssFile::Close(long long *retsz) {
  return ceph_posix_close(m_fd);
}

ssize_t CephOssFile::Read(off_t offset, size_t blen) {
  return XrdOssOK;
}

ssize_t CephOssFile::Read(void *buff, off_t offset, size_t blen) {
  int rc = ceph_posix_lseek(m_fd, offset, SEEK_SET);
  if (0 == rc) {
    return ceph_posix_read(m_fd, buff, blen);
  }
  return rc;
}

int CephOssFile::Read(XrdSfsAio *aoip) {
  return -ENOTSUP;
}

ssize_t CephOssFile::ReadRaw(void *buff, off_t offset, size_t blen) {
  return Read(buff, offset, blen);
}

int CephOssFile::Fstat(struct stat *buff) {
  return ceph_posix_fstat(m_fd, buff);
}

ssize_t CephOssFile::Write(const void *buff, off_t offset, size_t blen) {
  int rc = ceph_posix_lseek(m_fd, offset, SEEK_SET);
  if (0 == rc) {
    return ceph_posix_write(m_fd, buff, blen);
  }
  return rc;
}

int CephOssFile::Write(XrdSfsAio *aiop) {
  ssize_t rc = Write((void*)aiop->sfsAio.aio_buf,
                     aiop->sfsAio.aio_offset,
                     aiop->sfsAio.aio_nbytes);
  if (aiop->sfsAio.aio_nbytes == (size_t)rc) {
    aiop->Result = rc;
    aiop->doneWrite();
    return 0;
  } else {
    return rc;
  }
}

int CephOssFile::Fsync() {
  return ceph_posix_fsync(m_fd);
}

int CephOssFile::Ftruncate(unsigned long long len) {
  return ceph_posix_ftruncate(m_fd, len);
}
