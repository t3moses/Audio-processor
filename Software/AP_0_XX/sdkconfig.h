
// Including this macro allows timer alarms to call interrupt handlers directly,
// thereby reducing latency.

#define CONFIG_ESP_TIMER_SUPPORTS_ISR_DISPATCH_METHOD 1  // Or 0 to disable
