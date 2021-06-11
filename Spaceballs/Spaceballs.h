#pragma once

void canread(int canInterfaceOffset, int idOffset);
void chargercomms();
void inputdebug();
void currentlimit();
void sendcommand();
void dashupdate();
void resetwdog();
void outputdebug();
void CAB500();
void CAB300();
void handleVictronLynx();
int pgnFromCANId(int canId);
