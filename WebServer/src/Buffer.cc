#include "Buffer.h"
#include "../net/Socket.hpp"
#include <cerrno>
#include <sys/uio.h>

ssize_t Buffer::ReadFd(int fd, int *savedErrno) {
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = WritableBytes();
    //当buffer有足够空间，就直接写入buffer，不会写入extrabuf
    //当buffer没有足够空间，就先写入extrabuf，一是减少扩容的开销，二是防止一次传入大量数据的攻击
    vec[0].iov_base = WritePos();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = readv(fd, vec, iovcnt);
    if(n < 0) {
        // 读取socket缓冲区出错
        *savedErrno = errno;
    }
    else if(n <= writable) {
        // 读取的大小小于可写区域大小，直接移动写下标
        _writer_idx += n;
    }
    else {
        // 读取的大小大于可写区域大小，说明有部分写入了extra buffer，appedn就行
        _writer_idx = _buffer.size();
        Append(extrabuf, n - writable);
    }

    return n;
};