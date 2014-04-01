#include <mchck.h>

#include "power-meter.desc.h"

int active = 0;

static struct cdc_ctx cdc;

struct adc_queue_ctx core_temp, ext_temp, power1, power2;

static void
core_temp_done(uint16_t data, int error, void *cbdata)
{
        unsigned accum volt = adc_as_voltage(data);
        accum volt_diff = volt - 0.719k;
        accum temp_diff = volt_diff * (1000K / 1.715K);
        accum temp_deg = 25k - temp_diff;
        printf("core: raw=%u, temp=%.1k\r\n", data, temp_deg);
        active--;
}

static void
ext_temp_done(uint16_t data, int error, void *cbdata)
{
        accum v = adc_as_voltage(data);
        // linear approximation for -40oC to +85oC
        accum temp = (v - 1.8583) * 1000 / -11.67;
        printf("external: raw=%u, temp=%.1k\r\n", data, temp);
        active--;
}

static void
power1_done(uint16_t data, int error, void *cbdata)
{
        printf("power1: raw=%u\r\n", data);
        active--;
}

static void
power2_done(uint16_t data, int error, void *cbdata)
{
        printf("power2: raw=%u\r\n", data);
        active--;
}

static void
new_data(uint8_t *data, size_t len)
{
        bool start = false;

        for (; len > 0; ++data, --len) {
                switch (data[0]) {
                case '\r':
                        if (len > 1 && data[1] == '\n')
                                ++data, --len;
                        /* FALLTHROUGH */
                case '\n':
                        start = true;
                        break;
                }
        }

        if (!active && start) {
                active += 4;
                adc_queue_sample(&core_temp, ADC_TEMP, 0, core_temp_done, NULL);
                adc_queue_sample(&ext_temp, ADC_PTB2, 0, ext_temp_done, NULL);
                adc_queue_sample(&power1, ADC_PTB0, 0, power1_done, NULL);
                adc_queue_sample(&power2, ADC_PTB1, 0, power2_done, NULL);
        }

        cdc_read_more(&cdc);
}

void
init_vcdc(int config)
{
        cdc_init(new_data, NULL, &cdc);
        cdc_set_stdout(&cdc);
}

void
main(void)
{
        pin_mode(PIN_PTB0, PIN_MODE_MUX_ANALOG);
        pin_mode(PIN_PTB1, PIN_MODE_MUX_ANALOG);
        pin_mode(PIN_PTB2, PIN_MODE_MUX_ANALOG);
        pin_mode(PIN_PTB3, PIN_MODE_MUX_ANALOG);
        adc_init();
        usb_init(&cdc_device);
        sys_yield_for_frogs();
}
