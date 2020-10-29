void ANIM_HOR(uint8_t* sevenData, uint8_t state) {
  switch(state) {
        case 0: {
                    sevenData[0] = SEG_E | SEG_B;
                    sevenData[1] = SEG_E | SEG_B;
                    sevenData[2] = SEG_E | SEG_B;
                    sevenData[3] = SEG_E | SEG_B;
                    break;
                }
        case 1: {
                    sevenData[0] = SEG_F | SEG_C;
                    sevenData[1] = SEG_F | SEG_C;
                    sevenData[2] = SEG_F | SEG_C;
                    sevenData[3] = SEG_F | SEG_C;
                    break;
                }
        }
}
void ANIM_VER(uint8_t* sevenData, uint8_t state) {
  switch(state) {
        case 0: {
                    sevenData[0] = SEG_A;
                    sevenData[1] = SEG_D;
                    sevenData[2] = SEG_A;
                    sevenData[3] = SEG_D;
                    break;
                }
        case 1: {
                    sevenData[0] = SEG_D;
                    sevenData[1] = SEG_A;
                    sevenData[2] = SEG_D;
                    sevenData[3] = SEG_A;
                    break;
                }
        }
}

void ANIM_BLINK(uint8_t* sevenData, uint8_t state) {
  switch(state) {
        case 0: {
                    sevenData[0] = SEG_A | SEG_F | SEG_E | SEG_D;
                    sevenData[1] = 0x00;
                    sevenData[2] = 0x00;
                    sevenData[3] = SEG_A | SEG_B | SEG_C | SEG_D;
                    break;
                }
        case 1: {
                    sevenData[0] = 0x00;
                    sevenData[1] = 0x00;
                    sevenData[2] = 0x00;
                    sevenData[3] = 0x00;
                    break;
                }
        case 2: {
                    sevenData[0] = SEG_A | SEG_F | SEG_E | SEG_D | SEG_G;
                    sevenData[1] = SEG_G;
                    sevenData[2] = SEG_G;
                    sevenData[3] = SEG_A | SEG_B | SEG_C | SEG_D | SEG_G;
                    break;
                }
        case 3: {
                    sevenData[0] = SEG_A | SEG_F | SEG_E | SEG_D;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_A | SEG_B | SEG_C | SEG_D;
                    break;
                }
        }
}

void ANIM_WAIT(uint8_t* sevenData, uint8_t state) {
  switch(state) {
        case 0: {
                    sevenData[0] = SEG_A | SEG_F;
                    sevenData[1] = 0x00;
                    sevenData[2] = 0x00;
                    sevenData[3] = SEG_C | SEG_D;
                    break;
                }
        case 1: {
                    sevenData[0] = SEG_E | SEG_D;
                    sevenData[1] = 0x00;
                    sevenData[2] = 0x00;
                    sevenData[3] = SEG_A | SEG_B;
                    break;
                }

        }
}

void ANIM_CIRCLE (uint8_t* sevenData, uint8_t state)
{
    switch(state) {
        case 0: {
                    sevenData[0] = SEG_D | SEG_E | SEG_F;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_A | SEG_D | SEG_B | SEG_C;
                    break;
                }
        case 1: {
                    sevenData[0] = SEG_A | SEG_D | SEG_E | SEG_F;
                    sevenData[1] = SEG_D;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_A | SEG_D | SEG_B | SEG_C;
                    break;
                }
        case 2: {
                    sevenData[0] = SEG_A | SEG_D | SEG_E | SEG_F;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_D;
                    sevenData[3] = SEG_A | SEG_D | SEG_B | SEG_C;
                    break;
                }
        case 3: {
                    sevenData[0] = SEG_A | SEG_D | SEG_E | SEG_F;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_D | SEG_B | SEG_C;
                    break;
                }
        case 4: {
                    sevenData[0] = SEG_A | SEG_D | SEG_E | SEG_F;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_A | SEG_D | SEG_C;
                    break;
                }
        case 5: {
                    sevenData[0] = SEG_A | SEG_D | SEG_E | SEG_F;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_A | SEG_D | SEG_B;
                    break;
                }
        case 6: {
                    sevenData[0] = SEG_A | SEG_D | SEG_E | SEG_F;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_A | SEG_B | SEG_C;
                    break;
                }
        case 7: {
                    sevenData[0] = SEG_A | SEG_D | SEG_E | SEG_F;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_A;
                    sevenData[3] = SEG_A | SEG_D | SEG_B | SEG_C;
                    break;
                }
        case 8: {
                    sevenData[0] = SEG_A | SEG_D | SEG_E | SEG_F;
                    sevenData[1] = SEG_A;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_A | SEG_D | SEG_B | SEG_C;
                    break;
                }
        case 9: {
                    sevenData[0] = SEG_A | SEG_E | SEG_F;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_A | SEG_D | SEG_B | SEG_C;
                    break;
                }
        case 10: {
                    sevenData[0] = SEG_A | SEG_D | SEG_F;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_A | SEG_D | SEG_B | SEG_C;
                    break;
                }
        case 11: {
                    sevenData[0] = SEG_A | SEG_D | SEG_E;
                    sevenData[1] = SEG_A | SEG_D;
                    sevenData[2] = SEG_A | SEG_D;
                    sevenData[3] = SEG_A | SEG_D | SEG_B | SEG_C;
                    break;
                }
        default:
                {
                    sevenData[0] = SEG_G;
                    sevenData[1] = SEG_G;
                    sevenData[2] = SEG_G;
                    sevenData[3] = SEG_G;
                    break;

                }
    }
}
