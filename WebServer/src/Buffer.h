#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <memory>

//author: Haoyang Yang(Icepop)
//string_view: 是 C++ 17 新增的特性，它的本质是一个指针，它指向字符串的切片
//             它能够减少传参的拷贝，提高网络IO的效率      
//question: 那我用指针、引用不也能实现一样的效果？
//answer: 并非如此，因为指针和引用只能指向整个对象内存，很笨重，依赖于 '\0' 来判断字符串结尾。
//        而 string_view 顾名思义，它能够对原字符串进行“切片”，想看哪部分就看哪部分，轻量化。
class Buffer
{   
    using StringPiece = std::string_view;
public:
    static const size_t InitialSize = 1024;

    Buffer(const size_t& initialSize = InitialSize) : 
        _buffer(initialSize), _reader_idx(0), _writer_idx(0) 
        {}
    ~Buffer() = default;
    
    void Swap(Buffer &rhs) {
        _buffer.swap(rhs._buffer);
        std::swap(_reader_idx, rhs._reader_idx);
        std::swap(_writer_idx, rhs._writer_idx);
    }

    size_t ReadableBytes() const { return _writer_idx - _reader_idx; }

    size_t WritableBytes() const { return _buffer.size() - _writer_idx; }

    char *ReadPos() { return Begin() + _reader_idx; }

    char *WritePos() { return Begin() + _writer_idx; }

    /* brief: Equal to Write() and WriteAndPush() */
    void Append(const void *data, size_t len) {
        if(len == 0) return;
        EnsureWritableBytes(len);
        const char *pos = (const char*)data;
        std::copy(pos, pos + len, WritePos());
        HasWritten(len);
    }
    /* brief: Equal to WriteString() and  WriteStringAndPsuh() */
    void Append(const std::string_view &data) {
        Append(data.data(), data.size());
    }
    /* brief: Equal to WriteBuffer() and WriteBufferAndPush() */
    void Append(Buffer &data) {
        Append(data.ReadPos(), data.ReadableBytes());
    }

    void EnsureWritableBytes(size_t len) {
        if(len > WritableBytes()) MakeSpace(len);
        assert(len <= WritableBytes());
    }

    /* brief: Equal to MoveReadOffset(uint64_t step) */
    void Retrieve(size_t len) {
        assert(len <= ReadableBytes());
        _reader_idx += len;
    }

    void Read(void *buf, size_t len) {
        assert(len <= ReadableBytes());
        std::memcpy(buf, ReadPos(), len);
    }

    void ReadAndPop(void *buf, size_t len) {
        Read(buf, len);
        Retrieve(len);
    }

    std::string ReadAsString(size_t len) {
        assert(len <= ReadableBytes());
        std::string str(ReadPos(), len);
        return str;
    }

    std::string ReadAsStringAndPop(size_t len) {
        std::string str = ReadAsString(len);
        Retrieve(len);
        return str;
    }

    void Clear() {
        _reader_idx = 0;
        _writer_idx = 0;
    }

    char *FindCrlf() {
        char *res = (char*)memchr(ReadPos(), '\n', ReadableBytes());
        return res;
    }

    std::string Getline() {
        char* pos = FindCrlf();
        if(pos == NULL) return "";
        return ReadAsString(pos - ReadPos() + 1);
    }

    std::string GetlineAndPop() {
        std::string str = Getline();
        Retrieve(str.size());
        return str;
    }

    void shrink(size_t reserve) {
        Buffer other;
        other.EnsureWritableBytes(ReadableBytes() + reserve);
        other.Append(std::string_view(ReadPos(), static_cast<int>(ReadableBytes())));
        Swap(other);
    }

    ssize_t ReadFd(int fd, int *savedErrno);
private:
    char *Begin() { return &*(_buffer.begin()); }

    size_t ReadedBytes() const { return _reader_idx; }
    /* brief: Equal to MoveWriteOffset() */
    void HasWritten(size_t len) { _writer_idx += len; }

    void MakeSpace(size_t len) {
        if(len <= ReadedBytes() + WritableBytes()) {
            size_t readable = ReadableBytes();
            memcpy(Begin(), ReadPos(), readable);
            _reader_idx = 0;
            _writer_idx = _reader_idx + readable;
            assert(readable == ReadableBytes());
        }
        else {
            // 读过的空间加上可读的空间不够，就扩容
            _buffer.resize(_writer_idx + len);
        }
    }
private:
    std::vector<char> _buffer;
    size_t _reader_idx;
    size_t _writer_idx;
};