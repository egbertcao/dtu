
/* System sleep enable flag */
/*TRUE:sleep; FALSE: wakeup*/
extern void OC_SystemSleepEnable(BOOL enable);


/*Reset slient, don't print dump info*/
extern BOOL silentReset_Save(void);

#define CUSTOMER_APP_SEND_DATA_EVENT     (0x01)
#define CUSTOMER_APP_GET_DATA_EVENT     (0x02)
#define CUSTOMER_APP_SYS_WAKEUP_EVENT     (0x04)
#define CUSTOMER_APP_SYS_SLEEP_EVENT     (0x10)
#define CUSTOMER_APP_USB_CONNECT_EVENT     (0x20)
#define CUSTOMER_APP_USB_DISCONNECT_EVENT     (0x40)

#define CUSTOMER_APP_FLAGS_MASK    (CUSTOMER_APP_SEND_DATA_EVENT\
                                | CUSTOMER_APP_GET_DATA_EVENT \
                                | CUSTOMER_APP_SYS_WAKEUP_EVENT\
                                | CUSTOMER_APP_SYS_SLEEP_EVENT\
                                | CUSTOMER_APP_USB_CONNECT_EVENT\
                                | CUSTOMER_APP_USB_DISCONNECT_EVENT)
                                