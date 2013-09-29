//
// File stream.
//


#ifndef BSTONE_FILE_STREAM_H
#define BSTONE_FILE_STREAM_H


#include <fstream>

#include "bstone_istream.h"


namespace bstone {


// File stream.
class FileStream :
    public IStream
{
public:
    FileStream();

    FileStream(
        const std::string& file_name,
        StreamOpenMode open_mode = STREAM_OPEN_READ);

    bool open(
        const std::string& file_name,
        StreamOpenMode open_mode = STREAM_OPEN_READ);

    virtual void close();

    virtual bool is_open() const;

    virtual Sint64 get_size();

    virtual bool set_size(
        Sint64 size);

    virtual bool flush();

    virtual Sint64 seek(
        Sint64 offset,
        StreamSeekOrigin origin = STREAM_SEEK_BEGIN);

    virtual int read(
        void* buffer,
        int count);

    virtual bool write(
        const void* buffer,
        int count);

    virtual bool can_read() const;

    virtual bool can_seek() const;

    virtual bool can_write() const;

private:
    std::fstream stream_;
    Sint64 size_;
    Sint64 position_;
    bool can_read_;
    bool can_seek_;
    bool can_write_;
    bool need_sync_read_position_;
    bool need_sync_write_position_;

    FileStream(
        const FileStream& that);

    FileStream& operator=(
        const FileStream& that);
}; // class FileStream


} // namespace bstone


#endif // BSTONE_FILE_STREAM_H
