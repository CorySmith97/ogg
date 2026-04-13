
void notifications_push(Notification notification)
{
    arrins(notification_queue.list, 0, notification);
}

void notifications_update(void)
{
    for (s32 i = 0; i < arrlen(notification_queue.list); i++) {
        Notification *n = &notification_queue.list[i];
    }
}

void notifications_flush(void)
{
    s32 midpoint_x = GAME_WIDTH / 2;
    s32 midpoint_y = GAME_HEIGHT / 2;
    s32 len = arrlen(notification_queue.list);
    for (s32 i = 0; i < len; i++) {
        Notification *n = &notification_queue.list[i];
    }
}
