


int main(){

   int counter=0;
   struct sockaddr_in server;
   int sock;
   sock = socket(AF_INET , SOCK_STREAM , 0);
   server.sin_addr.s_addr = inet_addr("193.226.12.217");
   server.sin_family = AF_INET;
   server.sin_port = htons( 20232 );
   if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
   {
        cout<<"connect failed. Error";
   }

  char message[2];
    message[0]='f';
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            cout<<"Send failed";
        
        }
  return 0;
}    