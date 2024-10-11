#include "stdio.h"
#define uint16_t unsigned int
#define Queque_MaxLen     10

//事件类型
#define Message_InEvent   0x01
#define Message_OutEventb 0x02

//定义消息结构，便于下游设备从队列中读取所需数据
typedef struct Message
{
    uint16_t type;
    uint16_t msg;
}Message,*ms;

//定义队列状态
typedef enum
{
    OK,
    Error,
    Else_Ptr
}Status;

//中央设备消息结点链表
typedef struct NODE
{
    int type;
    int data; 
    void * next;
}NODE_TYP;
NODE_TYP Node;

//队列指针链表
typedef struct MyQuqu
{
    NODE_TYP *front;
    NODE_TYP *rear;
    uint16_t ququ_index;
}MQ;


//入队
Status EnQuqu(MQ *lhead,uint16_t data_type,uint16_t Data)
{
    if(lhead == NULL)return Error;
    NODE_TYP *temp = (NODE_TYP *)malloc(sizeof(NODE_TYP));
    lhead->rear->next = temp;
    temp->type = data_type;
    temp->data = Data;
    temp->next = NULL;
    lhead->rear = temp;
    lhead->ququ_index++;
    return OK;
}

//出队
Status DeQuqu(MQ *lhead,ms msg)
{
    if(lhead->front == lhead->rear)return Error;
    NODE_TYP *temp = lhead->front->next;
    msg->type = temp->type;
    msg->msg = temp->data;
    lhead->front->next = temp->next;
    lhead->front = temp->next;
    lhead->ququ_index--;
    if(lhead->rear == temp)lhead->rear = lhead->front;
    free(temp);                                         //在这里释放掉入队时分配的堆
    return OK;
}

//判断队列是否满
Status Q_Full(MQ *lhead)
{
    return lhead->ququ_index >= Queque_MaxLen? OK : Error;
}

//读取队列，将所需信息取出之后出队
Status Read_Q(MQ *lhead,ms msg,uint16_t data_type)
{
    NODE_TYP *temp = lhead->front->next;
    while(temp != NULL)
    {
        if(temp->type == data_type)
        {
            switch(data_type)
            {
                case Message_InEvent : 
                    DeQuqu(lhead,msg);
                    break;
                case Message_OutEventb:
                    DeQuqu(lhead,msg);
                    break;
                default:break;
            }
        }
        else temp = temp->next;
    }
}

//初始化队列
Status Quequ_Init(MQ *lhead)
{
    lhead->front = lhead->rear = lhead;
    lhead->front->next = NULL;
    return OK;
}

void main(void)
{
    MQ *myququ = (MQ *)malloc(sizeof(MQ));
}


