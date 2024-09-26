#ifndef WATERTEMP_CONTROL_H_
#define WATERTEMP_CONTROL_H_

void water_temp_control(int watertemp, int temperature_water){
	if(watertemp==1){ //65로 만들기
		if(temperature_water<65){
			WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
		}
		else{
			WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
		}
	}
	else if(watertemp==2){
		if(temperature_water<70){
			WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
		}
		else{
			WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
		}
			
	}
	else if(watertemp==3){
		if(temperature_water<75){
			WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
		}
		else{
			WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
		}
			
	}
	
}
void seat_temp_control(int seattemp, int temperature_seat){ 
	UART_printString(seattemp);
	UART_printString("   ");
	UART_printString(temperature_seat);
	UART_printString("\n");
	if(watertemp==1){ //65로 만들기
		if(temperature_seat<65){
			WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
			UART_printString("on");
			UART_printString("\n");
		}
		else{
			WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
			UART_printString("off");
			UART_printString("\n");
		}
	}
	else if(watertemp==2){
		if(temperature_seat<70){
			WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
		}
		else{
			WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
		}
				
	}
	else if(watertemp==3){
		if(temperature_seat<75){
			WATER_HEAT_PORT |= (1 << WATER_HEAT_PIN);
		}
		else{
			WATER_HEAT_PORT &= ~(1 << WATER_HEAT_PIN);
		}
				
	}
}


#endif /* WATERTEMP_CONTROL_H_ */

