#include "ByteArray.h"

ByteArray::ByteArray() :
    _buffer(nullptr),
    _pos(0),
    _length(0),
    _endian(ENDIAN_LITTLE),
    _flag(-1)
{
};

ByteArray::~ByteArray()
{
    if (this->_flag == 0 && this->_buffer) {
        delete[] this->_buffer;
        this->_buffer = nullptr;
        this->_flag = -1;
    }
};

ByteArray* ByteArray::create(size_t len)
{
    ByteArray* ba = new ByteArray();
    if (ba)
    {
        ba->_buffer = new char[len];
        ba->_length = len;
        ba->_flag = 0;
    }
    else {
        delete ba;
        ba = nullptr;
        return nullptr;
    }

    return ba;
}

ByteArray* ByteArray::createWithBuffer(char* buffer, size_t len, bool transferOwnerShip)
{
    ByteArray* ba = new ByteArray();
    if (ba)
    {
        ba->_buffer = buffer;
        ba->_length = len;
        if (transferOwnerShip)
            ba->_flag = 0;
        else
            ba->_flag = 1;
        return ba;
    }
    else {
        delete ba;
        ba = NULL;
        return NULL;
    }
    return ba;
}

bool ByteArray::readBool()
{
    bool b;
    char* p = (char*)&b;
    copyMemory(p, this->_buffer + this->_pos, sizeof(b));
    this->_pos += sizeof(b);
    return b;
}


void ByteArray::writeBool(bool value)
{
    char* p = (char*)&value;
    memcpy(this->_buffer + this->_pos, p, sizeof(value));
    this->_pos += sizeof(value);
}


short ByteArray::readShort()
{
    short n = 0;
    char* p = (char*)&n;
    copyMemory(p, this->_buffer + this->_pos, sizeof(n));
    this->_pos += 2;
    return n;
}


void ByteArray::writeShort(short value)
{
    char* p = (char*)&value;
    memcpy(this->_buffer + this->_pos, p, sizeof(value));
    this->_pos += 2;
}


int ByteArray::readInt() {
    int n;
    char* p = (char*)&n;
    copyMemory(p, this->_buffer + this->_pos, sizeof(n));
    this->_pos += 4;
    return n;
}


void ByteArray::writeInt(int value)
{
    char* p = (char*)&value;
    memcpy(this->_buffer + this->_pos, p, sizeof(value));
    this->_pos += 4;
}


unsigned int ByteArray::readUnsignedInt() {
    unsigned int n;
    char* p = (char*)&n;
    copyMemory(p, this->_buffer + this->_pos, sizeof(n));

    this->_pos += 4;
    return n;
}


void ByteArray::writeUnsignedInt(unsigned int value)
{
    char* p = (char*)&value;
    memcpy(this->_buffer + this->_pos, p, sizeof(value));
    this->_pos += 4;
}


unsigned short ByteArray::readUnsignedShort() {
    unsigned short n;
    char* p = (char*)&n;
    copyMemory(p, this->_buffer + this->_pos, sizeof(n));
    this->_pos += 2;
    return n;
}


void ByteArray::writeUnsignedShort(unsigned short value)
{
    char* p = (char*)&value;
    memcpy(this->_buffer + this->_pos, p, sizeof(value));
    this->_pos += 2;
}


char ByteArray::readByte() {
    signed char val = this->_buffer[this->_pos];
    if (val > 127) {
        val = val - 255;
    }
    this->_pos += 1;
    return val;
}


void ByteArray::writeByte(char value)
{
    char* p = (char*)&value;
    memcpy(this->_buffer + this->_pos, p, sizeof(value));
    this->_pos += 1;
}


unsigned char ByteArray::readUnsignedByte() {
    unsigned char val = this->_buffer[this->_pos];
    this->_pos += 1;
    return val;
}


void ByteArray::writeUnsignedChar(unsigned char value)
{
    char* p = (char*)&value;
    memcpy(this->_buffer + this->_pos, p, sizeof(value));
    this->_pos += 1;
}


long long ByteArray::readLongLong()
{
    long long n;
    char* p = (char*)&n;
    copyMemory(p, this->_buffer + this->_pos, sizeof(n));
    this->_pos += sizeof(n);
    return n;
}


void ByteArray::writeLongLong(long long value)
{
    char* p = (char*)&value;
    memcpy(this->_buffer + this->_pos, p, sizeof(value));
    this->_pos += sizeof(value);
}

std::string ByteArray::readString()
{
    int len = readUnsignedShort();
    return readString(len);
}

std::string ByteArray::readString(size_t len)
{
    char* value = new char[len + 1];

    value[len] = '\0';
    memcpy(value, this->_buffer + this->_pos, len);
    this->_pos += len;

    std::string str(value);
    delete[] value;
    value = nullptr;

    return str;
}

void ByteArray::writeString(const std::string& value)
{
    auto len = value.length();
    writeUnsignedShort(len);
    const char* p = value.c_str();
    memcpy(this->_buffer + this->_pos, p, len);
    this->_pos += len;
}

void ByteArray::copyMemory(void* to, const void* from, size_t len)
{
    char* t = (char*)to;
    char* f = (char*)from;
    if (_endian == ENDIAN_LITTLE)
    {
        memcpy(t, f, len);
    }
    else if (_endian == ENDIAN_BIG)
    {
        for (int i = len - 1; i >= 0; i--)
        {
            t[(len - 1) - i] = f[i];
        }
    }
}


const char* ByteArray::getBuffer()
{
    return this->_buffer;
}


void ByteArray::clear()
{
    memset(this->_buffer, 0, this->_length);
}


//0: little 1:big
int ByteArray::checkCPUEndian() {
    union w
    {
        int a;
        char b;
    } c;
    c.a = 1;
    return  (c.b == 1 ? ENDIAN_LITTLE : ENDIAN_BIG);
}


void ByteArray::setEndian(int n)
{
    this->_endian = n;
}


int ByteArray::getEndian()
{
    return this->_endian;
}


size_t ByteArray::getPosition()
{
    return this->_pos;
}


void ByteArray::setPosition(size_t pos)
{
    this->_pos = pos;
}


size_t ByteArray::getLength()
{
    return this->_length;
}


size_t ByteArray::getBytesAvailable()
{
    return this->_length - this->_pos;
}

