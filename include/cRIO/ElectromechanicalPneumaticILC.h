/*
 * Electromechanical and Pneumatic ILC functions.
 *
 * Developed for the Vera C. Rubin Observatory Telescope & Site Software Systems.
 * This product includes software developed by the Vera C.Rubin Observatory Project
 * (https://www.lsst.org). See the COPYRIGHT file at the top-level directory of
 * this distribution for details of code ownership.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _cRIO_ElectromechanicalPneumaticILC_h
#define _cRIO_ElectromechanicalPneumaticILC_h

#include <ILC/ILCBusList.h>
#include <Modbus/Buffer.h>

namespace LSST {
namespace cRIO {

/**
 * Class for communication with Electromechanical and Pneumatic ILCs. Those are
 * both in M1M3 for force actuator controls, as in M2 for stepper driven
 * actuator controls. The class handles requests for both, leaving it up to the
 * end users which methods will be implemented.
 *
 * Replies received from ILCs shall be processed with ILC::processResponse method.
 *
 * @defgroup M1M3_fa used on M1M3 force actuators
 * @defgroup M1M3_hp used on M1M3 hardpoints
 * @defgroup M2 used on M2 stepper motor actuators
 */
class ElectromechanicalPneumaticILC : public virtual ILC::ILCBusList {
public:
    /**
     * Populate responses for known ILC functions.
     *
     * @param bus ILC bus number (1..).
     */
    ElectromechanicalPneumaticILC(uint8_t bus);

    /**
     * Unicast command to command stepper motor moves.
     *
     * @param address ILC address
     * @param steps commanded steps
     *
     * @note processStepperForceStatus method is called to process replies
     *
     * @ingroup M1M3_hp
     * @ingroup M2
     */
    void setStepperSteps(uint8_t address, int8_t steps) { callFunction(address, 66, 1800, steps); }

    /**
     * Unicast Stepper motor ILC Force [N] and Status Request. ILC command
     * code 67 (0x43). Applies for M2 tangent and axials controllers, as well
     * as M1M3 hardpoints.
     *
     * @param address ILC address
     *
     * @note processStepperForceStatus method is called to process replies
     *
     * @ingroup M1M3_hp
     * @ingroup M2
     */
    void reportStepperForceStatus(uint8_t address) { callFunction(address, 67, 1800); }

    /**
     * Unicast command to set a single force actuator force offset.
     *
     * @param address ILC address
     * @param slewFlag DCA booster valve
     * @param primary primary actuator force offset (N)
     *
     * @ingroup M1M3_fa
     */
    void setSAAForceOffset(uint8_t address, bool slewFlag, float primary) {
        callFunction(address, 75, 1800, static_cast<uint8_t>(slewFlag ? 0xFF : 0x00),
                     Modbus::int24_t(primary * 1000));
    }

    /**
     * Unicast command to set a dual force actuator force offsets.
     *
     * @param address ILC address
     * @param slewFlag DCA booster valve
     * @param primary primary actuator force offset (N)
     * @param secondary secondary actuator force offset (N)
     *
     * @ingroup M1M3_fa
     */
    void setDAAForceOffset(uint8_t address, bool slewFlag, float primary, float secondary) {
        callFunction(address, 75, 1800, static_cast<uint8_t>(slewFlag ? 0xFF : 0x00),
                     Modbus::int24_t(primary * 1000), Modbus::int24_t(secondary * 1000));
    }

    /**
     * Reports force actuator status.
     *
     * @params address ILC address
     *
     * @ingroup M1M3_fa
     */
    void reportForceActuatorForceStatus(uint8_t address) { callFunction(address, 76, 1800); }

    /**
     * Unicast to set DCA Gain. ILC Command code 73 (0x49).
     *
     * @param address ILC address
     * @param orimaryGain axial booster valve gain
     * @param secondaryGain lateral booster valve gain
     *
     * @note no method is called for response, as this only sets the ILC paremeters
     *
     * @ingroup M1M3_fa
     */
    void setDCAGain(uint8_t address, float primaryGain, float secondaryGain) {
        callFunction(address, 73, 40000, primaryGain, secondaryGain);
    }

    /**
     * Read DCA Gain. ILC Command code 74 (0x4A).
     *
     * @param address ILC address
     *
     * @note processDCAGain method is called to process replies
     *
     * @ingroup M1M3_fa
     */
    void reportDCAGain(uint8_t address) { callFunction(address, 74, 2000); }

    /**
     * Unicast ADC Channel Offset and Sensitivity. ILC command code 81 (0x51).
     *
     * @param address ILC address
     * @param channel ADC channel (1-4)
     * @param offset ADC offset value
     * @param sensitivity ADC sensitivity value
     *
     * @note no method is called for response, as this only sets the ILC paremeters
     *
     * @ingroup M1M3_fa
     * @ingroup M2
     */
    void setOffsetAndSensitivity(uint8_t address, uint8_t channel, float offset, float sensitivity) {
        callFunction(address, 81, 36500, channel, offset, sensitivity);
    }

    /**
     * Read ILC calibration data. ILC command code 110 (0x6E).
     *
     * @param address ILC address
     *
     * @note processCalibrationData method is called to process replies
     *
     * @ingroup M1M3_fa
     * @ingroup M1M3_hp
     * @ingroup M2
     */
    void reportCalibrationData(uint8_t address) { callFunction(address, 110, 1800); }

    /**
     * Read ILC mezzanine pressure. ILC command code 119 (0x77).
     *
     * @param address ILC address
     *
     * @ingroup M1M3_hp
     */
    void reportMezzaninePressure(uint8_t address) { callFunction(address, 119, 1800); }

    /**
     * Unicast command to read hardpoint LVDT. ILC command 122 (0x7a).
     *
     * @param address ILC address
     *
     * @note processHardpointLVDT method is called to process replies
     *
     * @ingroup M1M3_hp
     */
    void reportHardpointLVDT(uint8_t address) { callFunction(address, 122, 400); }

protected:
    /**
     * Called when response from call to command unicast 66 (0x42) and 67
     * (0x43) is read.
     *
     * @param address returned from this ILC
     * @param status hardpoint Status
     * @param encoderPostion encoder position
     * @param loadCellForce measured load cell/actuator force
     *
     * @ingroup M2
     */
    virtual void processStepperForceStatus(uint8_t address, uint8_t status, int32_t encoderPostion,
                                           float loadCellForce) = 0;

    /**
     * Called when response from call to command 74 (0x4A) is read.
     *
     * @param address returned from this ILC
     * @param primaryGain axial booster gain value
     * @param secondaryGain lateral booster gain value
     *
     * @ingroup M1M3_fa
     */
    virtual void processDCAGain(uint8_t, float primaryGain, float secondaryGain) = 0;

    /**
     * Called when response from call to command 122 (0x7a) is read.
     *
     * @param address returned from this ILC
     * @param breakawayLVDT breakway LVDT value
     * @param displacementLVDT displacement LVDT value
     *
     * @ingroup M1M3_hp
     */
    virtual void processHardpointLVDT(uint8_t address, float breakawayLVDT, float displacementLVDT) = 0;

    /**
     * Called when response from call to command 210 or 220 is received. Called
     * for replies containing single axis (1 float for a single load cell) values.
     *
     * @param address returned from this ILC
     * @param status force actuator status mask
     * @param primaryLoadCellForce force in Newtons measured on the primary axis load cell
     *
     * @ingroup M1M3_fa
     */
    virtual void processSAAForceStatus(uint8_t address, uint8_t status, float primaryLoadCellForce) = 0;

    /**
     * Called when response from call to command 210 or 220 is received. Called
     * for replies containing double axis (2 floats for two load cells) values.
     *
     * @param address returned from this ILC
     * @param status force actuator status mask
     * @param primaryLoadCellForce force in Newtons measured on the primary axis load cell
     * @param secondaryLoadCellForce force in Newtons measured on the secondary axis load cell
     *
     * @ingroup M1M3_fa
     */
    virtual void processDAAForceStatus(uint8_t address, uint8_t status, float primaryLoadCellForce,
                                       float secondaryLoadCellForce) = 0;

    /**
     * Called when response from call to command 110 (0x6E) is read.
     *
     * @param address status returned from this ILC
     * @param mainADCK[4] main ADC calibration Kn
     * @param mainOffset[4] main sensor n offset
     * @param mainSensitivity[4] main sensor n sensitivity
     * @param backupADCK[4] backup ADC calibration Kn
     * @param backupOffset[4] backup sensor n offset
     * @param backupSensitivity[4] backup sensor n sensitivity
     *
     * @ingroup M1M3_fa
     * @ingroup M1M3_hp
     * @ingroup M2
     */
    virtual void processCalibrationData(uint8_t address, float mainADCK[4], float mainOffset[4],
                                        float mainSensitivity[4], float backupADCK[4], float backupOffset[4],
                                        float backupSensitivity[4]) = 0;

    /**
     * Called when response from call to command 119 is read.
     *
     * @param address status returned from this ILC
     * @param primaryPush primary push cylinder pressure (psi)
     * @param primaryPull primary pull cylinder pressure (psi)
     * @param secondaryPush secondary push cylinder pressure. Some value filled for single axis FAs. (psi)
     * @param secondaryPull secondary pull cylinder pressure. Some value filled for single axis FAs. (psi)
     *
     * @ingroup M1M3_fa
     */
    virtual void processMezzaninePressure(uint8_t address, float primaryPush, float primaryPull,
                                          float secondaryPush, float secondaryPull) = 0;
};

}  // namespace cRIO
}  // namespace LSST

#endif  //! _cRIO_ElectromechanicalPneumaticILC_h
