#include <wiringPi.h>

// PIN LED: il PIN 0 di wiringPi è BCM_GPIO 17.
// è necessario usare la numerazione BCM quando si esegue l'inizializzazione con wiringPiSetupSys
// quando si sceglie un PIN diverso, usare la numerazione BCM e
// aggiornare il comando Pagine delle proprietà - Eventi di compilazione - Evento di post-compilazione remota 
// che usa gpio export per la configurazione di wiringPiSetupSys
#define	LED	17

int main(void)
{
	wiringPiSetupSys();

	pinMode(LED, OUTPUT);

	while (true)
	{
		digitalWrite(LED, HIGH);  // Attivato
		delay(500); // ms
		digitalWrite(LED, LOW);	  // Disattivato
		delay(500);
	}
	return 0;
}