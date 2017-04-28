uint32_t array0[3] = {1, 1, 1};
uint32_t array1[3] = {1, 1, 1};
uint32_t array2[3] = {0, 0, 0};
uint32_t adresult[3] = {0};
uint32_t max[3] = {0, 0, 0};
uint32_t min[3] = {1000, 1000, 1000};
uint32_t dc[3] = {500, 500, 500};
uint32_t vpp[3] = {30, 30, 30};	
uint32_t precision[3] = {5, 5, 5};	
uint32_t old_fixed[3] = {0};
uint32_t new_fixed[3] = {0};
uint32_t bad_flag[3] = {0};
uint32_t STEPS = 0;
uint8_t ADXL345_FLAG = 0;
uint8_t BUF[8] = {0};

void step_counter(void)
{
	static uint8_t sampling_counter=0;
	uint8_t jtemp;

	ADXL345_FLAG = 0;
	Multiple_read_ADXL345();    //连续读出数据，存储在 buffer 中
	/*****采样滤波*****/
	for(jtemp = 0; jtemp <= 2; jtemp++)
	{		
		array2[jtemp] = array1[jtemp];
		array1[jtemp] = array0[jtemp];			
   		array0[jtemp] = BUF[2 * jtemp] + (BUF[2 * jtemp + 1] << 8);
   		
   		adresult[jtemp] = array0[jtemp] + array1[jtemp] + array2[jtemp];
  	 	adresult[jtemp] = adresult[jtemp] / 3;
		if(adresult[jtemp] > max[jtemp])
		{
			max[jtemp] = adresult[jtemp];
		}
		if(adresult[jtemp] < min[jtemp])
		{
			min[jtemp] = adresult[jtemp];
		}
	}
  	sampling_counter = sampling_counter + 1;
	/* 计算动态门限和动态精度 */
    if(sampling_counter >= 50)
    {               
      	sampling_counter = 0;			
		for(jtemp = 0; jtemp <= 2; jtemp++)
		{
			vpp[jtemp] = max[jtemp] - min[jtemp];
        	dc[jtemp] = min[jtemp] + (vpp[jtemp] >> 1);
			max[jtemp] = 0;
        	min[jtemp] = 1023;
			bad_flag[jtemp] = 0;
			if(vpp[jtemp] >= 160)
			{
				precision[jtemp] = vpp[jtemp] / 32; //8
			}
        	else if((vpp[jtemp] >= 50) && (vpp[jtemp] < 160))            
			{
				precision[jtemp] = 4;
			}
       		else if((vpp[jtemp] >= 15) && (vpp[jtemp] < 50))  
            {
				precision[jtemp] = 3;
			}  			
			else
       		{ 
          		precision[jtemp] = 2;
            	bad_flag[jtemp] = 1;
        	}
		}
  	}		
	/* 线性移位寄存器 */
	for(jtemp = 0; jtemp <= 2; jtemp++)
	{
		old_fixed[jtemp] = new_fixed[jtemp];

    	if(adresult[jtemp] >= new_fixed[jtemp])                         
    	{   
     		if((adresult[jtemp] - new_fixed[jtemp]) >= precision[jtemp])   
 			{
 				new_fixed[jtemp] = adresult[jtemp];
 			}
    	}
    	else if(adresult[jtemp] < new_fixed[jtemp])
   	 	{   
       		if((new_fixed[jtemp] - adresult[jtemp]) >= precision[jtemp])   
       		{
       			new_fixed[jtemp] = adresult[jtemp];
       		}
    	}
	}
	/* 动态门限判决 */
	if((vpp[0] >= vpp[1]) && (vpp[0] >= vpp[2]))
	{
		if((old_fixed[0] >= dc[0]) && (new_fixed[0] < dc[0]) && (bad_flag[0] == 0))        
		{
			STEPS = STEPS + 1;
		} 
	}
	else if((vpp[1] >= vpp[0]) && (vpp[1] >= vpp[2]))
	{
		if((old_fixed[1] >= dc[1]) && (new_fixed[1] < dc[1]) && (bad_flag[1] == 0))        
		{
			STEPS = STEPS + 1;
		}
	}
	else if((vpp[2] >= vpp[1]) && (vpp[2] >= vpp[0]))
	{
		if((old_fixed[2] >= dc[2]) && (new_fixed[2] < dc[2]) && (bad_flag[2] == 0))        
		{
			STEPS = STEPS + 1;
		}
	}
}