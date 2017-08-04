
### 高效率发送大数据
- 原理：通过在每次数据传输完成后继续传输后面剩余的数据和减少每次传输之间的间隔时间来提高发送效率，这样使得一个连接事件可以发送多个包（最多6个）。
```c
typedef struct blk_send_msg_s
{
	uint32_t start;			// send start offset
	uint32_t max_len;		// the total length of the data to be sent
	uint8_t	*pdata;
} blk_send_msg_t;

blk_send_msg_t	m_send_msg;

uint32_t ble_send(uint8_t *data, uint16_t len)
{
    ble_gatts_hvx_params_t hvx_params;

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = m_test.test_handle.value_handle;
    hvx_params.p_data = data;
    hvx_params.p_len  = &len;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(m_test.conn_handle, &hvx_params);
}

uint32_t send_data(void)
{
	uint8_t temp_len;
	uint32_t dif_value;
	uint32_t err_code = NRF_SUCCESS;
	uint8_t *pdata = m_send_msg.pdata;
	uint32_t start = m_send_msg.start;
	uint32_t max_len = m_send_msg.max_len;
	
	do {
		dif_value = max_len - start;
		temp_len = dif_value > 20? 20:dif_value;
		err_code = ble_send(pdata+start, temp_len);
		if(NRF_SUCCESS == err_code)
		{
			start += temp_len;
		}
	} while((NRF_SUCCESS == err_code) && (max_len - start) > 0);
	m_send_msg.start = start;
	
	return err_code;
}

uint32_t ble_send_data(uint8_t *pdata, uint32_t len)
{
	if(NULL == pdata || len <= 0)
	{
		return NRF_ERROR_INVALID_PARAM;
	}
	else
	{
		uint32_t	err_code = NRF_SUCCESS;
		m_send_msg.start = 0;
		m_send_msg.max_len = len;
		m_send_msg.pdata = pdata;
		
		err_code = send_data();
		
		return err_code;
	}
}

uint32_t ble_send_more_data(void)
{
	uint32_t err_code;
	uint32_t dif_value;
	
	dif_value = m_send_msg.max_len - m_send_msg.start;
	if(0 == dif_value || NULL == m_send_msg.pdata)
	{
		return NRF_SUCCESS;
	}
	else
	{
		err_code = send_data();
		
		return err_code;
	}
}

static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
	case BLE_EVT_TX_COMPLETE:	// 添加发送完成处理
	    ble_send_more_data();
    	    break;
	default:
            // No implementation needed.
            break;
    }
}

uint8_t g_data[500];
/*
初始化
for(uint32_t i = 0; i < 500; i++)
{
	g_data[i] = i;
}
*/
uint32_t test_send_more_data(void)
{
	uint32_t err_code = ble_send_data(g_data, 500);
	NRF_LOG_INFO("first send, err_code: %d\r\n", err_code);
	return err_code;
}
```