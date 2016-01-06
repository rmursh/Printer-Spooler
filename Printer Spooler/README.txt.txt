1) Compilation : Compile both files by running the follwoing commands in the directory they are stored
  
  #gcc -o {name_of_out_file_client} Print_Client.c -lrt -lpthread
  #gcc -o {name_of_out_file_server} Print_Server.c -lrt -lpthread

2) Running the spooler : Run the program using the following commands

   
  #./{name_of_out_file_server} // Monitors the server. Open another terminal
  #./{name_of_out_file_client} {ID} {PagesToPrint} {Duration} // All arguments as integers 
   
