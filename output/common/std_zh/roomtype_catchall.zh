enum roomtypes
{
	//0
	rtNONE, rtSPECIALITEM, rtPAYINFO, rtSECRETMONEY, rtGAMBLE,
	//5
	rtDOORREPAIR, rtPOTIONORHEART, rtGORIYA, rtLEVEL9, rtPOTION,
	//10
	rtSHOP, rtBOMBS, rtMOYL, rtTENRUPY, rtSTAIRWARP,
	//15
	rtGAN, rtWINGAME, rtITEMPOND, /*item ponds not implemented*/ rtMAGIC, rtSLASH, 
	//20
	rtARROWS, rtONEITEM, rtLAST
};

enum catchalltypes
{
	//0
	caNONE, caITEM, caPRICE, caAMOUNT, caGAMBLE,
	//5
	caSHOPID, caINFOSHOPID, caWARPRING, caPONDID, 
	//10
	caLAST
	
};

catchalltypes getCatchallType(mapdata m)
{
	switch(m->RoomType)
	{
		//no catchall
		case rtNONE:
		case rtPOTIONORHEART:
		case rtGORIYA:
		case rtLEVEL9:
		case rtTENRUPY:
		case rtGAN:
		case rtWINGAME:
		case rtMAGIC:
		case rtSLASH:
			return caNONE;
		
		case rtGAMBLE:
			return caGAMBLE;
		
		//item id
		case rtSPECIALITEM:
			return caITEM;
		
		case rtSECRETMONEY:
			return caAMOUNT;
		
		//shops
		case rtPOTION:
		case rtSHOP:
		case rtONEITEM:
			return caSHOPID;
		
		case rtPAYINFO:
			return caINFOSHOPID;
		
		//price
		case rtARROWS:
		case rtDOORREPAIR:
		case rtBOMBS:
		case rtMOYL: //the non-heart cost
			return caPRICE;
			
		case rtSTAIRWARP: return caWARPRING;
		case rtITEMPOND: return caPONDID;
		
		default:
			return caNONE;
		
	}
	
	
}