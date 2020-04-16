#!/bin/bash
# Userland tool to control Asus FanControl fans.


I2C_DEVICE="2" # See README.md for tricks on how to find this

I2C_ADDRESS="0x2a" # For a ASUS ROG Strix RTX2070 SUPER
PWM_REG_ADDDRESS="0x41" # For a ASUS ROG Strix RTX2070 SUPER 

# see fancontrol.8 for information about these values:
MINTEMP=40
MAXTEMP=80
MINSTART=25
MINSTOP=20
MINPWM=0
MAXPWM=200
AVERAGE=1
INTERVAL=2

DEBUG=""
LOG=""

function check_requirements() {
    if [[ ! -e "/dev/i2c-${I2C_DEVICE}" ]]; then
        echo >&2 "Can't find i2c device /dev/i2c-${I2C_DEVICE}. Run modprobe i2c_dev."
        exit 1;
    fi

    if [[ ! -w "/dev/i2c-${I2C_DEVICE}" ]]; then
        echo >&2 "We can't write to i2c device /dev/i2c-${I2C_DEVICE}."
        exit 1;
    fi

    command -v nvidia-smi >/dev/null 2>&1 || { 
        echo >&2 "Can't find nvidia-smi. Run apt install nvidia-config.";
        exit 1;
    }
    command -v i2cset >/dev/null 2>&1 || { 
        echo >&2 "Can't find i2cset. Run apt install i2c-tools.";
        exit 1;
    }
}

function get_gpu_temperature() {
    echo "$(nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader)"
}

function get_nb_gpu(){
    echo "$(nvidia-smi -L | wc -l)"
}

function set_pwm(){
    local pwm_val="${1}"
    if [[ ${pwm_val} -lt 0 ]]; then
        echo "can't set speed below 0"
        exit 1;
    fi
    if [[ ${pwm_val} -gt 255 ]]; then
        echo "can't set speed above 255"
        exit 1;
    fi
    if [ "$LOG" != "" ]; then
        echo "Setting GPU fan pwm to: ${pwm_val}"
    fi
    i2cset -y "${I2C_DEVICE}" "${I2C_ADDRESS}" "${PWM_REG_ADDDRESS}" "${pwm_val}"
}

function get_pwm(){
    local hex_val
    hex_val="$(i2cget -y "${I2C_DEVICE}" "${I2C_ADDRESS}" "${PWM_REG_ADDDRESS}")"
    echo $((hex_val))
}

function update_temp(){
    tval=$(get_gpu_temperature)
    # debug info



    mint=${MINTEMP}
    maxt=${MAXTEMP}
    minsa=${MINSTART}
    minso=${MINSTOP}
    minpwm=${MINPWM}
    maxpwm=${MAXPWM}
    tval="$(get_gpu_temperature)"
    pwmpval="$(get_pwm)"

    if [ "$DEBUG" != "" ]
    then
        echo "mint=$mint"
        echo "maxt=$maxt"
        echo "minsa=$minsa"
        echo "minso=$minso"
        echo "minpwm=$minpwm"
        echo "maxpwm=$maxpwm"
        echo "tval=$tval"
        echo "pwmpval=$pwmpval"
    fi

    if (( $tval <= $mint )); then
        if [ "$DEBUG" != "" ]; then
            echo "below min temp, use defined min pwm"
        fi
        pwmval=$minpwm
    elif (( $tval >= $maxt )); then
        if [ "$DEBUG" != "" ]; then
            echo "over max temp, use defined max pwm"
        fi
        pwmval=$maxpwm
    else
      # calculate the new value from temperature and settings
      pwmval=$(python -c "print(${tval}-${mint})*(${maxpwm}-${minso})/(${maxt}-${mint})+${minso}")
      if [ $pwmpval -eq 0 ]; then
          # if fan was stopped start it using a safe value
          set_pwm $minsa
      fi
    fi

    set_pwm $pwmval
    if [ $? -ne 0 ]; then
        echo "Error writing PWM value to $DIR/$pwmo" >&2
        restorefans 1
    fi

    if [ "$DEBUG" != "" ]; then
        echo "new pwmval=$pwmval"
    fi
}


function main() {
    while true; do
        update_temp
        sleep $INTERVAL 
    done
}

if [[ "$1" == "-d" ]]; then
    DEBUG="yaaas"
fi

if [[ "$1" == "-l" ]]; then
    LOG="yaaas"
fi

if [[ "$(get_nb_gpu)" -gt 1 ]] ; then
    echo "This script doesn't support multiple GPU (yet)"
    echo "Aborting."
fi

check_requirements

main
