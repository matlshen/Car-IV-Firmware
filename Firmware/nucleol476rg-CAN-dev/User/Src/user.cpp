#include "user.hpp"

/* RTOS prototypes */
void periodic_task1(void);
void regular_task1(void);

/* RTOS attributes */
osTimerAttr_t periodic_timer_attr =
{
    .name = "Periodic Task 1"
};
osTimerId_t periodic_timer_id = osTimerNew((osThreadFunc_t)periodic_task1, osTimerPeriodic, NULL, &periodic_timer_attr);

osThreadId_t regular_task_id;
uint32_t regular_task_buffer[1024];
StaticTask_t regular_task_control_block;
const osThreadAttr_t regular_task_attributes = {
    .name = "Regular Task",
    .cb_mem = &regular_task_control_block,
    .cb_size = sizeof(regular_task_control_block),
    .stack_mem = &regular_task_buffer[0],
    .stack_size = sizeof(regular_task_buffer),
    .priority = (osPriority_t) osPriorityAboveNormal,
};

osEventFlagsId_t regular_event = osEventFlagsNew(NULL);

/* CPP_UserSetup goes here */
void CPP_UserSetup(void) {

    CANNode node = CANNode(&hcan1, &hcan1);
    CANMessage msg1 = CANMessage(0x37, CAN_ID_STD, 8);
    CANNode::Start();
    CANNode::AddRxMessage(&msg1);

    regular_task_id = osThreadNew((osThreadFunc_t)regular_task1, NULL, &regular_task_attributes);

    osTimerStart(periodic_timer_id, 500);

    Logger::LogInfo("Program Started\n");
}

/* RTOS task definitions go here */
void periodic_task1(void) {
    Logger::LogInfo("Sending CAN message\n");
    CANMessage msg1 = CANMessage(0x37, CAN_ID_STD, 8);
    CANNode::Send(&msg1);

    osEventFlagsSet(regular_event, 0x1);
}

void regular_task1(void) {
    while (1) {
        osEventFlagsWait(regular_event, 0x1, osFlagsWaitAny, osWaitForever);

        Logger::LogInfo("Regular task\n");
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

        osDelay(100);
    }
}