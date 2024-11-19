
struct ClientInfo {
	int id;
	SOCKET clientSocket;
};

struct QueueBasket {
	uint requestType = LOGIN_TRY;
	SOCKET clientSocket;
	char clientName[16];
};