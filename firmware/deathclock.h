#define DC_mode_normal 0
#define DC_mode_pessimistic 1
#define DC_mode_optimistic 2
#define DC_mode_sadistic 3

#define DC_gender_male 0
#define DC_gender_female 1

#define DC_non_smoker 0
#define DC_smoker 1

#define BMI_Imperial 0
#define BMI_Metric 1
#define BMI_Direct 2

uint32_t date_diff ( uint8_t month1, uint8_t day1, uint8_t year1, uint8_t month2, uint8_t day2, uint8_t year2 );
uint8_t BodyMassIndex ( uint8_t unit, uint16_t height, uint16_t weight );
uint32_t ETD ( uint8_t DOB_month, 
	           uint8_t DOB_day, 
	           uint8_t DOB_year, 
	           uint8_t month, 
	           uint8_t day, 
	           uint8_t year, 
	           uint8_t Gender, 
	           uint8_t Mode, 
	           uint8_t BMI, 
	           uint8_t Smoker, 
	           uint8_t hour,
	           uint8_t min,
	           uint8_t sec);