/* Ross Baehr
   R@M 2017
   ross.baehr@gmail.com
*/

#include "interrupts/include/i2c_interrupt.h"

void I2CIntHandler(void) {
    //
    // Clear the I2C interrupt.
    //
    ROM_I2CMasterIntClear(I2C_LIB_DEVICE);

    //
    // Determine what to do based on the current state.
    //
    switch(int_state)
    {
        //
        // The idle state.
        //
        case STATE_IDLE:
        {
            //
            // There is nothing to be done.
            //
            break;
        }

        //
        // The state for the middle of a burst write.
        //
        case STATE_WRITE_NEXT:
        {
            //
            // Write the next byte to the data register.
            //
            ROM_I2CMasterDataPut(I2C_LIB_DEVICE, *buffer++);
            count--;

            //
            // Continue the burst write.
            //
            ROM_I2CMasterControl(I2C_LIB_DEVICE, I2C_MASTER_CMD_BURST_SEND_CONT);

            //
            // If there is one byte left, set the next state to the final write
            // state.
            //
            if(count == 1)
            {
                int_state = STATE_WRITE_FINAL;
            }

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the final write of a burst sequence.
        //
        case STATE_WRITE_FINAL:
        {
            //
            // Write the final byte to the data register.
            //
            ROM_I2CMasterDataPut(I2C_LIB_DEVICE, *buffer++);
            count--;

            //
            // Finish the burst write.
            //
            ROM_I2CMasterControl(I2C_LIB_DEVICE,
                             I2C_MASTER_CMD_BURST_SEND_FINISH);

            //
            // The next state is to wait for the burst write to complete.
            //
            int_state = STATE_SEND_ACK;

            //
            // This state is done.
            //
            break;
        }

        //
        // Wait for an ACK on the read after a write.
        //
        case STATE_WAIT_ACK:
        {
            //
            // See if there was an error on the previously issued read.
            //
            if(ROM_I2CMasterErr(I2C_LIB_DEVICE) == I2C_MASTER_ERR_NONE)
            {
                //
                // Read the byte received.
                //
                ROM_I2CMasterDataGet(I2C_LIB_DEVICE);

                //
                // There was no error, so the state machine is now idle.
                //
                int_state = STATE_IDLE;

                //
                // This state is done.
                //
                break;
            }

            //
            // Fall through to STATE_SEND_ACK.
            //
        }

        //
        // Send a read request, looking for the ACK to indicate that the write
        // is done.
        //
        case STATE_SEND_ACK:
        {
            //
            // Put the I2C master into receive mode.
            //
            ROM_I2CMasterSlaveAddrSet(I2C_LIB_DEVICE, address, true);

            //
            // Perform a single byte read.
            //
            ROM_I2CMasterControl(I2C_LIB_DEVICE, I2C_MASTER_CMD_SINGLE_RECEIVE);

            //
            // The next state is the wait for the ack.
            //
            int_state = STATE_WAIT_ACK;

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for a single byte read.
        //
        case STATE_READ_ONE:
        {
            //
            // Put the I2C master into receive mode.
            //
            ROM_I2CMasterSlaveAddrSet(I2C_LIB_DEVICE, address, true);

            //
            // Perform a single byte read.
            //
            ROM_I2CMasterControl(I2C_LIB_DEVICE, I2C_MASTER_CMD_SINGLE_RECEIVE);

            //
            // The next state is the wait for final read state.
            //
            int_state = STATE_READ_WAIT;

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the start of a burst read.
        //
        case STATE_READ_FIRST:
        {
            //
            // Put the I2C master into receive mode.
            //
            ROM_I2CMasterSlaveAddrSet(I2C_LIB_DEVICE, address, true);

            //
            // Start the burst receive.
            //
            ROM_I2CMasterControl(I2C_LIB_DEVICE,
                             I2C_MASTER_CMD_BURST_RECEIVE_START);

            //
            // The next state is the middle of the burst read.
            //
            int_state = STATE_READ_NEXT;

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the middle of a burst read.
        //
        case STATE_READ_NEXT:
        {
            //
            // Read the received character.
            //
            *buffer++ = ROM_I2CMasterDataGet(I2C_LIB_DEVICE);
            count--;

            //
            // Continue the burst read.
            //
            ROM_I2CMasterControl(I2C_LIB_DEVICE,
                             I2C_MASTER_CMD_BURST_RECEIVE_CONT);

            //
            // If there are two characters left to be read, make the next
            // state be the end of burst read state.
            //
            if(count == 2)
            {
                int_state = STATE_READ_FINAL;
            }

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the end of a burst read.
        //
        case STATE_READ_FINAL:
        {
            //
            // Read the received character.
            //
            *buffer++ = ROM_I2CMasterDataGet(I2C_LIB_DEVICE);
            count--;

            //
            // Finish the burst read.
            //
            ROM_I2CMasterControl(I2C_LIB_DEVICE,
                             I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

            //
            // The next state is the wait for final read state.
            //
            int_state = STATE_READ_WAIT;

            //
            // This state is done.
            //
            break;
        }

        //
        // This state is for the final read of a single or burst read.
        //
        case STATE_READ_WAIT:
        {
            //
            // Read the received character.
            //
            *buffer++  = ROM_I2CMasterDataGet(I2C_LIB_DEVICE);
            count--;

            //
            // The state machine is now idle.
            //
            int_state = STATE_IDLE;

            //
            // This state is done.
            //
            break;
        }
    }
}

