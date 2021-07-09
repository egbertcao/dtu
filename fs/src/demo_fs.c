#include "oc_pcac_fs.h"

#define TEST_FILE_NAME "/1.txt"
#define NEW_FILE_NAME "/2.txt"
#define TEST_DIR_NAME "C:/test123"

int oc_write_file(char *filename, char *buf)
{
     size_t bytes;
    int ret = 0;
    
    unsigned int fd = oc_fs_fopen(filename, "wb+"); // create file
    if (fd < 0)
    {
        OC_UART_LOG_Printf("%s: open [%s] failed!\r\n", __func__, filename);
        return -1;
    }

    bytes = oc_fs_fwrite(buf, 1, strlen(buf), fd);
    if (bytes <= 0)
    {
        OC_UART_LOG_Printf("%s: write file error or buf is empty!\r\n", __func__);
        return -1;
    }
    else
    {
        OC_UART_LOG_Printf("%s: write success, bytes - %d\r\n", __func__, bytes);
    }
    
    oc_fs_fclose(fd);
    return bytes;
}

int oc_read_file(char *filename, char *buf)
{
    size_t bytes;
    char buf_temp[512] = {0};
    unsigned int fd = oc_fs_fopen(filename, "rb");
    if (fd < 0)
    {
        OC_UART_LOG_Printf("%s: open [%s] failed!\r\n", __func__, filename);
        return -1;
    }
    bytes = oc_fs_fread(buf_temp, 1, sizeof(buf_temp), fd);
    if (bytes <= 0)
    {
        OC_UART_LOG_Printf("%s: read file error or file is empty!\r\n", __func__);
        return -1;
    }
    else
    {
        memcpy(buf, buf_temp, bytes);
        OC_UART_LOG_Printf("%s: read success! buf - %s, bytes - %d\r\n", __func__, buf, bytes);
    }
    oc_fs_fclose(fd);
    return bytes;
}

int oc_fs_demo(void)
{
    size_t bytes;
    
    char temp[1024]={"test txt"};
    char buf[1024] = {0};
    int offset = 0;
    int ret = 0;
    
    unsigned int fd = oc_fs_fopen(TEST_FILE_NAME, "wb+"); // create file
    if (fd < 0)
    {
        OC_UART_LOG_Printf("%s: open [%s] failed!\r\n", __func__, TEST_FILE_NAME);
        return -1;
    }

    bytes = oc_fs_fwrite(temp, 1, strlen(temp), fd);
    if (bytes <= 0)
    {
        OC_UART_LOG_Printf("%s: write file error or buf is empty!\r\n", __func__);
        return -1;
    }
    else
    {
        OC_UART_LOG_Printf("%s: write success, bytes - %d\r\n", __func__, bytes);
    }
    
    ret = oc_fs_fseek(fd, 0, SEEK_SET);
    if (ret < 0)
    {
        OC_UART_LOG_Printf("%s: file fseek error!\r\n", __func__);
        return -1;
    }
    
    bytes = oc_fs_fread(buf, 1, sizeof(buf), fd);
    if (bytes <= 0)
    {
        OC_UART_LOG_Printf("%s: read file error or file is empty!\r\n", __func__);
        return -1;
    }
    else
    {
        OC_UART_LOG_Printf("%s: read success! buf - %s, bytes - %d\r\n", __func__, buf, bytes);
    }

    offset = oc_fs_ftell(fd);
    OC_UART_LOG_Printf("%s: file offset - %d\r\n", __func__, offset);

    ret = oc_fs_fseek(fd, 10, SEEK_CUR);
    if (ret < 0)
    {
        OC_UART_LOG_Printf("%s: file fseek error!\r\n", __func__);
        return -1;
    }

    offset = oc_fs_ftell(fd);
    OC_UART_LOG_Printf("%s: file offset - %d\r\n", __func__, offset);

    oc_fs_fclose(fd);

    ret = oc_fs_rename(TEST_FILE_NAME, NEW_FILE_NAME);
    if (ret < 0)
    {
        OC_UART_LOG_Printf("%s: rename file error!\r\n", __func__);
        return -1;
    }

    fd = oc_fs_fopen(NEW_FILE_NAME, "rb");
    if (fd <= 0)
    {
        OC_UART_LOG_Printf("%s: open [%s] failed!\r\n", __func__, NEW_FILE_NAME);
        return -1;
    }

    bytes = oc_fs_fread(buf, 1, sizeof(buf), fd);
    if (bytes <= 0)
    {
        OC_UART_LOG_Printf("%s: read file error or file is empty!\r\n", __func__);
        return -1;
    }
    else
    {
        OC_UART_LOG_Printf("%s: read success! buf - %s, bytes - %d\r\n", __func__, buf, bytes);
    }

    oc_fs_fclose(fd);

    //oc_fs_remove(NEW_FILE_NAME);

    ret = oc_fs_mkdir(TEST_DIR_NAME);
    if (ret < 0)
    {
        OC_UART_LOG_Printf("%s: mkdir error!\r\n", __func__);
        return -1;
    }
    OC_UART_LOG_Printf("%s: mkdir success!\r\n", __func__);

    return 0;
    
}
