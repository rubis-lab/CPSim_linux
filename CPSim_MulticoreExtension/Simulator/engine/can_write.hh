		case 1:
		{
			CAN_Msg *can_msg = new CAN_Msg(cur_node->finish_time_ECU, 1, 0x7FF, whole_task_functions[1]->data[0], whole_task_functions[1]->data[0], cur_node->task->task_name);
			insert_can_msg(&waiting_data, can_msg);
		}
			break;

		case 4:
		{
			CAN_Msg *can_msg = new CAN_Msg(cur_node->finish_time_ECU, 1, 0x7FE, whole_task_functions[4]->data[0], whole_task_functions[4]->data[1], cur_node->task->task_name);
			insert_can_msg(&waiting_data, can_msg);
		}
			break;
