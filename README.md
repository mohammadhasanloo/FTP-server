# CN-CA1-FTP-Server
In this project, a simplified version of ftp has been implemented. This program consists of two parts: `Client` and `Server`. The server is responsible for serving files to the client.<br/>
How does this protocol work?<br/>
Communication requires two different channels to send and receive data. The first channel, which is called the command channel, is the channel through which commands and responses are sent. The other channel, which is called the data channel, is the channel that has the task of moving data.
