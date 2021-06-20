#include "oc_pcac_ftp.h"

#define TEST_IPADDR "58.246.1.50"
#define TEST_PORT 6521
#define TEST_USER_NAME "FTP_TST"
#define TEST_PASSWD "FTPTST_0320"
#define TEST_DIR "zyq_test"

int oc_ftp_demo(void)
{
    CPUartLogPrintf("%s ENTER...", __func__);
    int ret = 0;
    if (OC_Ftp_Config(0, 0, 0, "I") < 0)
    {
        CPUartLogPrintf("%s:OC_Ftp_Config error!", __func__);
        return -1;
    }

    if (OC_Ftp_New_Session(TEST_IPADDR, TEST_PORT, TEST_USER_NAME, TEST_PASSWD) < 0)
    {
        CPUartLogPrintf("%s: OC_Ftp_New_Session error!", __func__);
        return -1;
    }

    // make dir
    if (OC_Ftp_Newfolder(TEST_DIR) < 0)
    {
        CPUartLogPrintf("%s: make dir error!", __func__);
        return -1;
    }
    else
        CPUartLogPrintf("%s: make dir success!", __func__);
    //remove dir
    if (OC_Ftp_Deletefolder(TEST_DIR) < 0)
    {
        CPUartLogPrintf("%s: remove dir error!", __func__);
        return -1;
    }
    else
        CPUartLogPrintf("%s: remove dir success!", __func__);

    if (OC_Ftp_List("/") < 0)
    {
        CPUartLogPrintf("%s: list dir error!", __func__);
        return -1;
    }
    else
        CPUartLogPrintf("%s: list dir success!", __func__);

    // get file
    //OC_Ftp_Dowload_File("/jiang.txt","/zzz.txt",0);

    // put file
    //OC_Ftp_Upload_File("/z1.txt", "/zzz.txt",0);

    OC_Ftp_Dowload_FiletoURC("/jiang.txt", 0);
    return 0;
}
