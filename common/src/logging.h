#ifndef __LOGGING_H__
#define __LOGGING_H__
/*
  <0-7>          Logging severity level
  alerts         Immediate action needed           (severity=1)
  critical       Critical conditions               (severity=2)
  debugging      Debugging messages                (severity=7)
  discriminator  Establish MD-Console association
  emergencies    System is unusable                (severity=0)
  errors         Error conditions                  (severity=3)
  filtered       Enable filtered logging
  guaranteed     Guarantee console messages
  informational  Informational messages            (severity=6)
  notifications  Normal but significant conditions (severity=5)
  warnings       Warning conditions                (severity=4)
*/

enum logging_level {
    LOGGING_LEVEL_ALERTS,
    LOGGING_LEVEL_CRITICAL,
    LOGGING_LEVEL_ERRORS,
    LOGGING_LEVEL_NOTIFICATIONS,
    LOGGING_LEVEL_INFORMATIONAL,
    LOGGING_LEVEL_LAST /* should always be in last */
    
};

#define logging_define(level, fs, ...)					\
    do {								\
    void (* logging_message )(enum logging_level level);\
    ASSIGN_FUNC_PTR("logging_message", logging_message);\
    logging_message(level);						\
    printf(fs, ##__VA_ARGS__);						\
    printf("\n");							\
    } while(0)

#define logging_define_static(level, fs, ...)					\
    do {								\
    logging_message(level);						\
    printf(fs, ##__VA_ARGS__);						\
    printf("\n");							\
    } while(0)



#define logging_alerts(fs, ...)						\
    logging_define(LOGGING_LEVEL_ALERTS, fs, ##__VA_ARGS__)

#define logging_critical(fs, ...)					\
    logging_define(LOGGING_LEVEL_CRITICAL, fs, ##__VA_ARGS__)

#define logging_errors(fs, ...)						\
    logging_define(LOGGING_LEVEL_ERRORS, fs, ##__VA_ARGS__)

#define logging_notifications(fs, ...)					\
    logging_define(LOGGING_LEVEL_NOTIFICATIONS, fs, ##__VA_ARGS__)

#define logging_informational(fs, ...)					\
    logging_define(LOGGING_LEVEL_INFORMATIONAL, fs, ##__VA_ARGS__)

;

#endif /* __LOGGING_H__ */
