#include "std.zh"

generic script WebSocketScript
{
    void run()
    {
        printf("Connecting to websocket.\n");
        websocket ws = Game->LoadWebSocket("ws://ws.ifelse.io");
        ws->Own();
        bool hasSentMessage = false;

        while (true)
        {
            if (ws->State == WEBSOCKET_STATE_CONNECTING)
            {
                Waitframe();
                continue;
            }
            else if (ws->State == WEBSOCKET_STATE_CLOSED)
            {
                char32 error[0];
                ws->GetError(error);
                printf("Failed to connect: %s\n", error);
                break;
            }

            if (ws->State != WEBSOCKET_STATE_OPEN)
            {
                printf("Failed to connect: %l\n", ws->State);
                break;
            }

            if (!hasSentMessage)
            {
                printf("Connected!\n");
                ws->Send("hello world!");
                ws->Send("domo arigato!", WEBSOCKET_MESSAGE_TYPE_BINARY);
                hasSentMessage = true;
            }

            while (ws->HasMessage)
            {
                int message_ptr = ws->Receive();
                int type = ws->MessageType;
                if (type == WEBSOCKET_MESSAGE_TYPE_TEXT)
                    printf("got text message: %s\n", message_ptr);
                else if (type == WEBSOCKET_MESSAGE_TYPE_BINARY)
                {
                    int len = SizeOfArray(message_ptr);
                    printf("got binary message of length: %d\n", len);
                    for (int i = 0; i < len; ++i)
                        printf("%d: %l\n", i, message_ptr[i]);
                }
                else
                    printf("got message, but with unknown type: %l\n", type);
            }

            Waitframe();
        }
    }
}
