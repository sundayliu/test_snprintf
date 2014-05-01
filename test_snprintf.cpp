#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>

#ifdef _MSC_VER
#	include<io.h>
#	define snprintf _snprintf
#	define vsnprintf	_vsnprintf
#	define write	_write
#	define STDOUT_FILENO 1
#else
#	include <sys/types.h>
#	include <sys/uio.h>
#	include <unistd.h>
#endif

//////////////////////////////////////////////
// Output:
//	VC6:
//		ret:-1
//		buf:0123xxxxxxxxxx
// GCC:
//		ret:10
//		buf:012
/////////////////////////////////////////////
void test_1(){
	char buf[16];
	int ret;
	memset(buf,'x',sizeof(buf)); // fill with 'x'
	buf[(sizeof(buf)-sizeof(buf[0]))-1] = '\0';
	ret = snprintf(buf,4,"%s","0123456789");
	printf("ret:%d\n", ret);
	printf("buf:%s\n",buf);
}

void write_hello_v0(const char* name){
	char* pbuf = 0;
	int size;

	// Get required size, and allocate enough memory
	size = snprintf(0,0,"Hello, %s!\n",name);
	pbuf = (char*)malloc(size + 1);

	// do the formatting
	snprintf(pbuf,size+1,"Hello, %s!\n",name);

	// write formatted string
	write(STDOUT_FILENO,pbuf,size);

	free(pbuf);
}


// --[OUTPUT(GCC)]------------------------------------------------------------
// Hello, sign!
// Hello, jeffhung!
// Hello, Honorificabilitudinitatibus!
// --[OUTPUT(VC6)]------------------------------------------------------------
// (crashed)
// --[OUTPUT(VS2005)]------------------------------------------------------------
// Hello, sign!
// Hello, jeffhung!
// Hello, Honorificabilitudinitatibus!
void test_write_hello_v0(){
	write_hello_v0("sign"); // 4 chars:total 13 chars when write
	write_hello_v0("jeffhung"); // 8 chars:total 17 chars when write
	write_hello_v0("Honorificabilitudinitatibus"); // 27 chars:total 36 chars when write
}



void write_hello_v1(const char* name){
	char* pbuf = 0;
    int size = 0;
    int len;
    do {
        size += 16;
        printf("[DEBUG] size == %d\n", size);
        // Allocate a buffer, don't know whether it is big enough or not
        pbuf = (char*)realloc(pbuf, size); // will do malloc if pbuf is NULL
        // ------------------------------------------------------------------
        // Do the formatting
        // ------------------------------------------------------------------
        // MSDN:
        // Let len be the length of the formatted data string (not including
        // the terminating null).
        // - If len < count, then len characters are stored in buffer,
        //                   a null-terminator is appended,
        //                   and len is returned.
        // - If len = count, then len characters are stored in buffer,
        //                   no null-terminator is appended,
        //                   and len is returned.
        // - If len > count, then count characters are stored in buffer,
        //                   no null-terminator is appended,
        //                   and a negative value is returned.
        // ------------------------------------------------------------------
        // Since snprintf in VC may not append a null-terminator, we pass
        // (size - 1) as the 2nd parameter and reserve the last buffer
        // element for appending the null-terminator by our self.
        // ------------------------------------------------------------------
        len = snprintf(pbuf, (size - 1), "Hello, %s!\n", name);
        printf("[DEBUG] len == %d\n", len);
    } while (len < 0);
    pbuf[len] = '\0';
    // Write formatted string
    write(STDOUT_FILENO, pbuf, len);
    // Free allocated memory
    free(pbuf);
}

// --[OUTPUT(GCC)]------------------------------------------------------------
// [DEBUG] size == 16
// [DEBUG] len == 13
// Hello, sign!
// [DEBUG] size == 16
// [DEBUG] len == 17
// Hello, jeffhun[DEBUG] size == 16
// [DEBUG] len == 36
// Hello, Honorif
// --[OUTPUT(VC6)]------------------------------------------------------------
// [DEBUG] size == 16
// [DEBUG] len == 13
// Hello, sign!
// [DEBUG] size == 16
// [DEBUG] len == -1
// [DEBUG] size == 32
// [DEBUG] len == 17
// Hello, jeffhung!
// [DEBUG] size == 16
// [DEBUG] len == -1
// [DEBUG] size == 32
// [DEBUG] len == -1
// [DEBUG] size == 48
// [DEBUG] len == 36
// Hello, Honorificabilitudinitatibus!
void test_write_hello_v1(){
	write_hello_v1("sign");                        //  4 chars: total 13 chars when write
    write_hello_v1("jeffhung");                    //  8 chars: total 17 chars when write
    write_hello_v1("Honorificabilitudinitatibus"); // 27 chars: total 36 chars when write
}


/** Write a "Hello, <name>!/n" message to file descriptor STDOUT_FILENO. */
void write_hello_v2(const char* name)
{
    char  buf[16];
    char* pbuf      = buf;
    int   pbuf_size = sizeof(buf);
    int   len       = 0;
    int   again     = 0;
    printf("[DEBUG] name == \"%s\"\n", name);
    do {
        if (again) {
#ifdef _MSC_VER
            pbuf_size += sizeof(buf);
#else
            pbuf_size = len + 1;
#endif
            pbuf = (pbuf == buf) ? (char *)malloc(pbuf_size)
				: (char *)realloc(pbuf, pbuf_size);
        }
        printf("[DEBUG] pbuf_size == %d\n", pbuf_size);
        len = snprintf(pbuf, pbuf_size, "Hello, %s!\n", name);
        printf("[DEBUG] len == %d\n", len);
    } while (again = ((len < 0) || (pbuf_size <= len)));
#ifdef _MSC_VER
    pbuf[len] = '\0';
#endif
    printf("[DEBUG] {%d} %s", len, pbuf); // to verify the null-terminator
    write(STDOUT_FILENO, pbuf, len);
    if (pbuf != buf) {
        printf("[DEBUG] free pbuf\n");
        free(pbuf);
    }
}

// --[OUTPUT(GCC)]------------------------------------------------------------
// [DEBUG] name == "sign"
// [DEBUG] pbuf_size == 16
// [DEBUG] len == 13
// [DEBUG] {13} Hello, sign!
// Hello, sign!
// [DEBUG] name == "jeffhung"
// [DEBUG] pbuf_size == 16
// [DEBUG] len == 17
// [DEBUG] pbuf_size == 18
// [DEBUG] len == 17
// [DEBUG] {17} Hello, jeffhung!
// Hello, jeffhung!
// [DEBUG] free pbuf
// [DEBUG] name == "Honorificabilitudinitatibus"
// [DEBUG] pbuf_size == 16
// [DEBUG] len == 36
// [DEBUG] pbuf_size == 37
// [DEBUG] len == 36
// [DEBUG] {36} Hello, Honorificabilitudinitatibus!
// Hello, Honorificabilitudinitatibus!
// [DEBUG] free pbuf
// --[OUTPUT(VC6)]------------------------------------------------------------
// [DEBUG] name == "sign"
// [DEBUG] pbuf_size == 16
// [DEBUG] len == 13
// [DEBUG] {13} Hello, sign!
// Hello, sign!
// [DEBUG] name == "jeffhung"
// [DEBUG] pbuf_size == 16
// [DEBUG] len == -1
// [DEBUG] pbuf_size == 32
// [DEBUG] len == 17
// [DEBUG] {17} Hello, jeffhung!
// Hello, jeffhung!
// [DEBUG] free pbuf
// [DEBUG] name == "Honorificabilitudinitatibus"
// [DEBUG] pbuf_size == 16
// [DEBUG] len == -1
// [DEBUG] pbuf_size == 32
// [DEBUG] len == -1
// [DEBUG] pbuf_size == 48
// [DEBUG] len == 36
// [DEBUG] {36} Hello, Honorificabilitudinitatibus!
// Hello, Honorificabilitudinitatibus!
// [DEBUG] free pbuf
void test_write_hello_v2()
{
    write_hello_v2("sign");                        //  4 chars: total 13 chars when write
    write_hello_v2("jeffhung");                    //  8 chars: total 17 chars when write
    write_hello_v2("Honorificabilitudinitatibus"); // 27 chars: total 36 chars when write
}


std::string strprintf(const char* fmt, ...)
{
    char    buf[16];
    char*   pbuf      = buf;
    int     pbuf_size = sizeof(buf);
    int     len       = 0;
    int     again     = 0;
    va_list ap;
    va_start(ap, fmt);
    do {
        if (again) {
#ifdef _MSC_VER
            pbuf_size += sizeof(buf);
#else
            pbuf_size = len + 1;
#endif
            pbuf = (char*)((pbuf == buf) ? (char *)malloc(pbuf_size)
				: (char *)realloc(pbuf, pbuf_size));
        }
        printf("[DEBUG] pbuf_size == %d\n", pbuf_size);
        len = vsnprintf(pbuf, pbuf_size, fmt, ap);
        printf("[DEBUG] len == %d\n", len);
    } while (again = ((len < 0) || (pbuf_size <= len)));
#ifdef _MSC_VER
    pbuf[len] = '\0';
#endif
    printf("[DEBUG] {%d} %s", len, pbuf); // to verify the null-terminator
    std::string str(pbuf);
    if (pbuf != buf) {
        printf("[DEBUG] free pbuf\n");
        free(pbuf);
    }
    return str;
}
void write_hello_v3(const char* name)
{
    // 9 chars: counting ending /n,
    //          but not counting %s replacement and null-terminator
    std::string hello = strprintf("Hello, %s!\n", name);
    write(STDOUT_FILENO, hello.c_str(), hello.length());
}

// --[OUTPUT(C99)]------------------------------------------------------------
// [DEBUG] pbuf_size == 16
// [DEBUG] len == 13
// [DEBUG] {13} Hello, sign!
// Hello, sign!
// [DEBUG] pbuf_size == 16
// [DEBUG] len == 17
// [DEBUG] pbuf_size == 18
// [DEBUG] len == 17
// [DEBUG] {17} Hello, jeffhung!
// [DEBUG] free pbuf
// Hello, jeffhung!
// [DEBUG] pbuf_size == 16
// [DEBUG] len == 36
// [DEBUG] pbuf_size == 37
// [DEBUG] len == 36
// [DEBUG] {36} Hello, Honorificabilitudinitatibus!
// [DEBUG] free pbuf
// Hello, Honorificabilitudinitatibus!
// --[OUTPUT(VC)]-------------------------------------------------------------
// [DEBUG] pbuf_size == 16
// [DEBUG] len == 13
// [DEBUG] {13} Hello, sign!
// Hello, sign!
// [DEBUG] pbuf_size == 16
// [DEBUG] len == -1
// [DEBUG] pbuf_size == 32
// [DEBUG] len == 17
// [DEBUG] {17} Hello, jeffhung!
// [DEBUG] free pbuf
// Hello, jeffhung!
// [DEBUG] pbuf_size == 16
// [DEBUG] len == -1
// [DEBUG] pbuf_size == 32
// [DEBUG] len == -1
// [DEBUG] pbuf_size == 48
// [DEBUG] len == 36
// [DEBUG] {36} Hello, Honorificabilitudinitatibus!
// [DEBUG] free pbuf
// Hello, Honorificabilitudinitatibus!
void test_write_hello_v3()
{
    write_hello_v3("sign");                        //  4 chars: total 13 chars when write
    write_hello_v3("jeffhung");                    //  8 chars: total 17 chars when write
    write_hello_v3("Honorificabilitudinitatibus"); // 27 chars: total 36 chars when write
}



int main(int argc, char* argv[]){
	//test_1();
	//test_write_hello_v0();
	//test_write_hello_v1();
	//test_write_hello_v2();
	test_write_hello_v3();
	return 0;
}