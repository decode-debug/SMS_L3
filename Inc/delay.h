/*
 * delay.h
 *
 *  Created on: 1 lis 2022
 *      Author: Admin
 */

#ifndef DELAY_H_
#define DELAY_H_

void DelayTick(void);        // aktualizacja czasu opoznienia
void Delay(unsigned int);    // ustawienie czasu opoznienia i zawieszenie dzialania programu
void Delay_ms(unsigned int); // ustawienie czasu opoznienia i zawieszenie dzialania programu (argument okresla liczbe ms)

#endif /* DELAY_H_ */
