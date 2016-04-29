Chat Client

Functionality that interacts with the user works as described in the instructions, however priave messages are never recieved by anyone and are effectively lost in the ether.
When a message is sent all clients listening to the multicast rrecieve the message. When an announce is recieved the IP information of that user is added to map with std::string keys and a stored sockaddr_in as the value. This value is checked when sending private messages in order to find the appropriate IP address.
