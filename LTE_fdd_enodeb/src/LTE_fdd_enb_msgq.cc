#line 2 "LTE_fdd_enb_msgq.cc" // Make __FILE__ omit the path
/*******************************************************************************

    Copyright 2013-2014 Ben Wojtowicz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************

    File: LTE_fdd_enb_msgq.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 message queues.

    Revision History
    ----------    -------------    --------------------------------------------
    11/10/2013    Ben Wojtowicz    Created file
    11/23/2013    Ben Wojtowicz    Fixed a bug with receive size.
    01/18/2014    Ben Wojtowicz    Added ability to set priorities.
    06/15/2014    Ben Wojtowicz    Omitting path from __FILE__.

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_interface.h"
#include "LTE_fdd_enb_msgq.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/


/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/******************/
/*    Callback    */
/******************/
LTE_fdd_enb_msgq_cb::LTE_fdd_enb_msgq_cb()
{
}
LTE_fdd_enb_msgq_cb::LTE_fdd_enb_msgq_cb(FuncType f, void* o)
{
    func = f;
    obj  = o;
}
void LTE_fdd_enb_msgq_cb::operator()(LTE_FDD_ENB_MESSAGE_STRUCT *msg)
{
    return (*func)(obj, msg);
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_msgq::LTE_fdd_enb_msgq(std::string         _msgq_name,
                                   LTE_fdd_enb_msgq_cb cb)
{
    msgq_name = _msgq_name;
    callback  = cb;
    prio      = 0;
    pthread_create(&rx_thread, NULL, &receive_thread, this);
}
LTE_fdd_enb_msgq::LTE_fdd_enb_msgq(std::string         _msgq_name,
                                   LTE_fdd_enb_msgq_cb cb,
                                   uint32              _prio)
{
    msgq_name = _msgq_name;
    callback  = cb;
    prio      = _prio;
    pthread_create(&rx_thread, NULL, &receive_thread, this);
}
LTE_fdd_enb_msgq::~LTE_fdd_enb_msgq()
{
    try
    {
        LTE_fdd_enb_msgq::send(msgq_name.c_str(),
                               LTE_FDD_ENB_MESSAGE_TYPE_KILL,
                               LTE_FDD_ENB_DEST_LAYER_ANY,
                               NULL,
                               0);
        sleep(1);
    }catch(boost::interprocess::interprocess_exception &){
        // Intentionally do nothing
    }

    // Cleanup thread
    pthread_cancel(rx_thread);
    pthread_join(rx_thread, NULL);
}

/**********************/
/*    Send/Receive    */
/**********************/
void LTE_fdd_enb_msgq::send(const char                    *mq_name,
                            LTE_FDD_ENB_MESSAGE_TYPE_ENUM  type,
                            LTE_FDD_ENB_DEST_LAYER_ENUM    dest_layer,
                            LTE_FDD_ENB_MESSAGE_UNION     *msg_content,
                            uint32                         msg_content_size)
{
    boost::interprocess::message_queue mq(boost::interprocess::open_only, mq_name);

    LTE_fdd_enb_msgq::send(&mq, type, dest_layer, msg_content, msg_content_size);
}
void LTE_fdd_enb_msgq::send(boost::interprocess::message_queue *mq,
                            LTE_FDD_ENB_MESSAGE_TYPE_ENUM       type,
                            LTE_FDD_ENB_DEST_LAYER_ENUM         dest_layer,
                            LTE_FDD_ENB_MESSAGE_UNION          *msg_content,
                            uint32                              msg_content_size)
{
    LTE_FDD_ENB_MESSAGE_STRUCT *msg = NULL;

    msg = new LTE_FDD_ENB_MESSAGE_STRUCT;

    msg->type       = type;
    msg->dest_layer = dest_layer;
    if(msg_content != NULL)
    {
        memcpy(&msg->msg, msg_content, msg_content_size);
    }

    mq->send(&msg, sizeof(msg), 0);
}
void* LTE_fdd_enb_msgq::receive_thread(void *inputs)
{
    LTE_fdd_enb_msgq           *msgq = (LTE_fdd_enb_msgq *)inputs;
    LTE_FDD_ENB_MESSAGE_STRUCT *msg  = NULL;
    struct sched_param          priority;
    std::size_t                 rx_size;
    uint32                      prio;
    bool                        not_done = true;

    // Set priority
    if(msgq->prio != 0)
    {
        // FIXME: verify
        priority.sched_priority = prio;
        pthread_setschedparam(msgq->rx_thread, SCHED_FIFO, &priority);
    }

    // Open the message_queue
    boost::interprocess::message_queue mq(boost::interprocess::open_only,
                                          msgq->msgq_name.c_str());

    while(not_done)
    {
        // Wait for a message
        mq.receive(&msg, sizeof(msg), rx_size, prio);

        // Process message
        if(sizeof(msg) == rx_size)
        {
            switch(msg->type)
            {
            case LTE_FDD_ENB_MESSAGE_TYPE_KILL:
                not_done = false;
                delete msg;
                break;
            default:
                msgq->callback(msg);
                break;
            }
        }else{
            // FIXME: Use print_debug_msg
            printf("ERROR %s Invalid message size received: %u\n",
                   msgq->msgq_name.c_str(),
                   rx_size);
        }
    }

    return(NULL);
}
