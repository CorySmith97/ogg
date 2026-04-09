#ifndef NOTIFICATION_H
#define NOTIFICATION_H

typedef enum {
    NOTIFICATION_ERROR,
    NOTIFICATION_WARNING,
    NOTIFICATION_INFO,
    NOTIFICATION_COUNT,
} NotificationTag;

typedef struct {
    NotificationTag tag;
    s32     time_in_ms;
    String8 msg;
} Notification;

void notifications_flush(void);

#endif // NOTIFICATION_H
