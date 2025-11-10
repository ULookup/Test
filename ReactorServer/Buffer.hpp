#include <vector>
#include <string>
#include <memory>
#include <cstring>
#include <cstdint>
#include <cassert>
class Buffer
{
public:
    const static uint64_t InitialSize = 1024;

    Buffer(const uint64_t &initialSize = InitialSize) 
        : _buffer(initialSize), _read_idx(0), _write_idx(0) 
        {}
    ~Buffer() = default;
    /* brief: 获取缓冲区头地址 */
    char *Begin() { return &*(_buffer.begin()); }
    /* brief: 获取已读区域大小 */
    uint64_t ReadedBytes() const { return _write_idx; }
    /* brief: 获取可读区域大小 */
    uint64_t ReadAbleBytes() const { return _write_idx - _read_idx; }
    /* brief: 获取可写区域大小 */
    uint64_t WriteAbleBytes() const { return _buffer.size() - _write_idx; }
    /* brief: 获取读位置 */
    char *ReadPos() { return Begin() + _read_idx; }
    /* brief: 获取写位置 */
    char *WritePos() { return Begin() + _write_idx; }
    /* brief: 读位置向后移动 */
    void MoveReadOffset(uint64_t step) {
        assert(step <= ReadAbleBytes());
        _read_idx += step;
    }
    /* brief: 写位置向后移动 */
    void MoveWriteOffset(uint64_t step) {
        assert(step <= WriteAbleBytes());
        _write_idx += step;
    }
    /* brief: 确保空间足够 */
    void EnsureWriteSpace(uint64_t step) {
        if(step <= WriteAbleBytes()) return;
        if(step <= WriteAbleBytes() + ReadedBytes()) {
            //将数据移动到起始位置
            uint64_t rsz = ReadAbleBytes();
            std::copy(ReadPos(), ReadPos() + rsz, Begin());
            _read_idx = 0;
            _write_idx = rsz;
        } else {
            //空间不够，扩容
            _buffer.resize(_write_idx + step);
        }
    }
    /* brief: 写入数据 */
    void Write(const void* data, uint64_t len) {
        EnsureWriteSpace(len);
        const char* d = (const char*)data;
        std::copy(d, d + len, WritePos());
    }
    void WriteAndPush(const void* data, uint64_t len) {
        Write(data,len);
        MoveWriteOffset(len);
    }
    void WriteString(const std::string &data) {
        return Write(data.c_str(), data.size());
    }
    void WriteStringAndPush(const std::string &data) {
        WriteString(data);
        MoveWriteOffset(data.size());
    }
    void WriteBuffer(Buffer &data) {
        return Write(data.ReadPos(), data.ReadAbleBytes());
    }
    void WriteBufferAndPush(Buffer &data) {
        WriteBuffer(data);
        MoveWriteOffset(data.ReadAbleBytes());
    }
    /* brief: 读取数据 */
    void Read(void* buf, uint64_t len) {
        assert(len <= ReadAbleBytes());
        std::copy(ReadPos(), ReadPos() + len, (char*)buf);
    }
    void ReadAndPop(void *buf, uint64_t len) {
        Read(buf, len);
        MoveReadOffset(len);
    }
    std::string ReadAsString(uint64_t len) {
        assert(len <= ReadAbleBytes());
        std::string str;
        str.resize(len);
        Read(&str[0], len);
        return str;
    }
    std::string ReadAsStringAndPop(uint64_t len) {
        std::string str = ReadAsString(len);
        MoveReadOffset(len);
        return str;
    }
    /* brief: 清空缓冲区 */
    void Clear() {
        _read_idx = 0;
        _write_idx = 0;
    }
    /* brief: 读取一行数据 */
    char *FindCrlf() {
        char* res = (char*)memchr(ReadPos(), '\n', ReadAbleBytes());
        return res;
    }
    std::string Getline() {
        char* pos = FindCrlf();
        if(pos == NULL) return "";
        return ReadAsString(pos - ReadPos() + 1);
    }
    std::string GetlineAndPop() {
        std::string str = Getline();
        MoveReadOffset(str.size());
        return str;
    }
private:
    std::vector<char> _buffer;
    uint64_t _read_idx;
    uint64_t _write_idx;
};


const uint64_t Buffer::InitialSize;
