#ifndef LSA08
#define LSA08

struct lineSensor {int address; int uartPort; float distanceFromCenter; 
			float DAngle; float SAngle; int junctionPin; int UARTPin;};

char getLineSensorData(struct lineSensor ls);
signed int readLineSensor(struct lineSensor ls);
struct point finePos(struct lineSensor ls, float lineTheta, struct point junctionPoint, 
			struct point curPos, float curHeading);
void initLineSensor(struct lineSensor ls);
void initLineSensor(struct lineSensor ls, void (*junctionInterruptHandler)(void));

#endif
