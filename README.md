# DioSearchTool
A simple fast search utility for Linux desktop written in GTK.
You can use it independently in any Desktop Environment.
It was tested on Debian 12.

# What you can do with DioSearchTool
   1. Search for files and folders on your system.
   2. Option for case sensitive search.
   3. Option for searching regular files only (omiting directories).
   4. Option for recursive search (include subdirectories).
   5. Option to exclude HOME directory from search (useful for whole system search).
   6. Highlights the text in the search results.
   7. Open the file.
   8. Open with file.
   9. Open file location.
   10. Specify your own default app for opening files and the default file manager.

# Installation/Usage
  1. Open a terminal and run:

		 ./configure

  2. if all went well then run:

		 make
		 sudo make install
		 
		 (if you just want to test it then run: make run)
		
  4. Run the application:
  
		 diosearchtool

# Screenshots
 
![Alt text](https://github.com/DiogenesN/diosearchtool/blob/main/diosearchtool.png)
 
![Alt text](https://github.com/DiogenesN/diosearchtool/blob/main/diosearchtool2.png)

![Alt text](https://github.com/DiogenesN/diosearchtool/blob/main/diosearchtool3.png)

That's it!

 Make sure you have the following packages installed:

		make
		pkgconf
		libgtk-4-dev

# Support

   My Libera IRC support channel: #linuxfriends
   
   Email: nicolas.dio@protonmail.com

