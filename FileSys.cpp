// CPSC 3500: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <cmath>
#include <sys/types.h>
#include <sys/socket.h>
using namespace std;

#define LINE_END "\\r\\n"

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

// mounts the file system
void FileSys::mount(int sock) {
	bfs.mount();
	curr_dir = 1; //by default current directory is home directory, in disk block #1
	fs_sock = sock; //use this socket to receive file system operations from the client and send back response messages
}

// unmounts the file system
void FileSys::unmount() {
	bfs.unmount();
	close(fs_sock);
}

// make a directory
void FileSys::mkdir(const char* name)
{
	short newblock = bfs.get_free_block();
	if (newblock == 0) {
		err_msg = "505 Disk is full";
		sendExit(err_msg, 0, msg);
		return;
	}

	if (strlen(name) > MAX_FNAME_SIZE) {
		err_msg = "504 File name is too long";
		sendExit(err_msg, 0, msg);
		return;
	}

	dirblock_t curr_dir_block;
	bfs.read_block(curr_dir, &curr_dir_block);

	if (curr_dir_block.num_entries > MAX_DIR_ENTRIES) {
		err_msg = "506 Directory is full";
		sendExit(err_msg, 0, msg);
		return;
	}

	for (int i = 0; i < curr_dir_block.num_entries; i++) {
		if (isDirectory(curr_dir_block.dir_entries[i].block_num)) {
			if ((string)curr_dir_block.dir_entries[i].name == (string)name) {
				err_msg = "502 File exists";
				sendExit(err_msg, 0, msg);
				return;
			}
		}
	}

	for (int i = 0; i < MAX_FNAME_SIZE + 1; i++)
		curr_dir_block.dir_entries[curr_dir_block.num_entries].name[i] = name[i];
	curr_dir_block.dir_entries[curr_dir_block.num_entries].block_num = newblock;

	dirblock_t dir_block;
	dir_block.magic = DIR_MAGIC_NUM;
	dir_block.num_entries = 0;
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
		dir_block.dir_entries[i].block_num = 0;
	// write new dir to block
	bfs.write_block(newblock, (void*)&dir_block);

	curr_dir_block.num_entries++;
	// update curr directory
	bfs.write_block(curr_dir, (void*)&curr_dir_block);

	err_msg = "200 OK";
	sendExit(err_msg, 0, msg);
}

// switch to a directory
void FileSys::cd(const char* name)
{
	dirblock_t curr_dir_block;
	bfs.read_block(curr_dir, &curr_dir_block);
	bool exists = false;
	bool file = false;
	for (int i = 0; i < curr_dir_block.num_entries; i++) {
		if ((string)curr_dir_block.dir_entries[i].name == (string)name) {
			exists = true;
			if (isDirectory(curr_dir_block.dir_entries[i].block_num)) {
				curr_dir = curr_dir_block.dir_entries[i].block_num;
				file = false;
				break;
			}
			else
				file = true;
		}
	}
	if (!exists) {
		err_msg = "503 File does not exist";
		sendExit(err_msg, 0, msg);
		return;
	}
	if (file) {
		err_msg = "500 File is not a directory";
		sendExit(err_msg, 0, msg);
		return;
	}

	err_msg = "200 OK";
	sendExit(err_msg, 0, msg);

}

// switch to home directory
void FileSys::home()
{
	curr_dir = 1;
	err_msg = "200 OK";
	sendExit(err_msg, 0, msg);

}

// remove a directory
void FileSys::rmdir(const char* name)
{
	dirblock_t curr_dir_block;
	bfs.read_block(curr_dir, &curr_dir_block);

	dirblock_t target_block;
	short target_block_num;
	int index;
	bool exists = false;
	bool file = false;
	for (int i = 0; i < curr_dir_block.num_entries; i++) {
		if ((string)curr_dir_block.dir_entries[i].name == (string)name) {
			exists = true;
			if (isDirectory(curr_dir_block.dir_entries[i].block_num)) {
				target_block_num = curr_dir_block.dir_entries[i].block_num;
				bfs.read_block(target_block_num, &target_block);
				index = i;
				file = false;
				break;
			}
			else
				file = true;
		}
	}
	if (!exists) {
		err_msg = "503 File does not exist";
		sendExit(err_msg, 0, msg);
		return;
	}
	if (file && exists) {
		err_msg = "500 File is not a directory";
		sendExit(err_msg, 0, msg);
		return;
	}
	if (target_block.num_entries > 0) {
		err_msg = "507 Directory is not empty\n";
		sendExit(err_msg, 0, msg);
		return;
	}

	// if want to remove the last entry
	if (target_block_num == curr_dir_block.dir_entries[curr_dir_block.num_entries].block_num) {
		// free block with dir
		bfs.reclaim_block(target_block_num);
	}
	else {
		for (int i = index; i < curr_dir_block.num_entries; i++) {
			curr_dir_block.dir_entries[i] = curr_dir_block.dir_entries[i + 1];
		}
		// free block with dir
		bfs.reclaim_block(target_block_num);

	}
	curr_dir_block.num_entries--;
	// update curr directory
	bfs.write_block(curr_dir, (void*)&curr_dir_block);

	err_msg = "200 OK";
	sendExit(err_msg, 0, msg);
}

// list the contents of current directory
void FileSys::ls()
{
	msg = "";
	dirblock_t curr_dir_block;
	bfs.read_block(curr_dir, &curr_dir_block);
	if (curr_dir_block.num_entries > 0) {
		for (int i = 0; i < curr_dir_block.num_entries; i++) {
			if (isDirectory(curr_dir_block.dir_entries[i].block_num)) {
				msg += curr_dir_block.dir_entries[i].name;
				msg += "/ ";
			} else {
				msg += curr_dir_block.dir_entries[i].name;
				msg += " ";
			}
		}
		cout << endl;
	}

	err_msg = "200 OK";
	sendExit(err_msg, 1, msg);
}

// create an empty data file
void FileSys::create(const char* name)
{
	short newblock = bfs.get_free_block();
	if (newblock == 0) {
		err_msg = "505 Disk is full";
		sendExit(err_msg, 0, msg);
		return;
	}

	if (strlen(name) > MAX_FNAME_SIZE) {
		err_msg = "504 File name is too long";
		sendExit(err_msg, 0, msg);
		return;
	}

	dirblock_t curr_dir_block;
	bfs.read_block(curr_dir, &curr_dir_block);

	if (curr_dir_block.num_entries > MAX_DIR_ENTRIES) {
		err_msg = "506 Directory is full";
		sendExit(err_msg, 0, msg);
		return;
	}

	for (int i = 0; i < curr_dir_block.num_entries; i++) {
		if (!isDirectory(curr_dir_block.dir_entries[i].block_num)) {
			if ((string)curr_dir_block.dir_entries[i].name == (string)name) {
				err_msg = "502 File exists";
				sendExit(err_msg, 0, msg);
				return;
			}
		}
	}

	for (int i = 0; i < MAX_FNAME_SIZE + 1; i++)
		curr_dir_block.dir_entries[curr_dir_block.num_entries].name[i] = name[i];
	curr_dir_block.dir_entries[curr_dir_block.num_entries].block_num = newblock;

	inode_t inode_block;
	inode_block.magic = INODE_MAGIC_NUM;
	inode_block.size = 0;
	for (int i = 0; i < MAX_DATA_BLOCKS; i++)
		inode_block.blocks[i] = 0;

	// write new inode to block
	bfs.write_block(newblock, (void*)&inode_block);

	curr_dir_block.num_entries++;
	// update curr directory
	bfs.write_block(curr_dir, (void*)&curr_dir_block);

	err_msg = "200 OK";
	sendExit(err_msg, 0, msg);
}

// append data to a data file
void FileSys::append(const char* name, const char* data)
{
	dirblock_t curr_dir_block;
	bfs.read_block(curr_dir, &curr_dir_block);

	inode_t target_block;
	short target_block_num;
	bool exists = false;
	bool file = true;

	for (int i = 0; i < curr_dir_block.num_entries; i++) {
		if ((string)curr_dir_block.dir_entries[i].name == (string)name) {
			exists = true;
			if (!isDirectory(curr_dir_block.dir_entries[i].block_num)) {
				target_block_num = curr_dir_block.dir_entries[i].block_num;
				bfs.read_block(target_block_num, &target_block);
			}
			else
				file = false;
		}
	}
	if (!exists) {
		err_msg = "503 File does not exist";
		sendExit(err_msg, 0, msg);
		return;
	}
	if (!file) {
		err_msg = "501 File is a directory";
		sendExit(err_msg, 0, msg);
		return;
	}
	if (target_block.size + strlen(data) > MAX_FILE_SIZE) {
		err_msg = "508 Append exceeds maximum file size";
		sendExit(err_msg, 0, msg);
		return;
	}
	// if file is empty or all previous datablocks are fully used
	if (target_block.size % BLOCK_SIZE == 0) {
		double bytes_used = (double)strlen(data) / BLOCK_SIZE;
		int blocks_to_allocate = ceil(bytes_used);

		datablock_t data_block[MAX_DATA_BLOCKS];
		short free_blocks[MAX_DATA_BLOCKS];
		// check if there is enough disk space
		for (int i = 0; i < blocks_to_allocate; i++) {
			short newblock = bfs.get_free_block();
			if (newblock == 0) {
				err_msg = "505 Disk is full";
				sendExit(err_msg, 0, msg);
				return;
			}
			free_blocks[i] = newblock;
		}

		for (int i = 0; i < blocks_to_allocate; i++) {
			target_block.blocks[i] = free_blocks[i];

			int count = 0;
			while (data[count + i * BLOCK_SIZE] && count < BLOCK_SIZE) {
				data_block[i].data[count] = data[count + i * BLOCK_SIZE];
				count++;
			}

			bfs.write_block(free_blocks[i], (void*)&data_block[i]);
		}
		target_block.size = strlen(data);
	} else { // file is not empty and still has space in last block
		double bytes_used = (double)target_block.size / BLOCK_SIZE;
		int last_block_num = ceil(bytes_used) - 1;
		short free_blocks[MAX_DATA_BLOCKS];

		// if data requires more blocks to be allocated
		if (strlen(data) - ((last_block_num + 1) * BLOCK_SIZE - target_block.size) > 0) {
			int data_left = strlen(data) - ((last_block_num + 1) * BLOCK_SIZE - target_block.size);
			bytes_used = (double)data_left / BLOCK_SIZE;
			int blocks_to_allocate = ceil(bytes_used);
			// check if there is enough disk space
			for (int i = 0; i < blocks_to_allocate; i++) {
				short newblock = bfs.get_free_block();
				if (newblock == 0) {
					err_msg = "505 Disk is full";
					sendExit(err_msg, 0, msg);
					return;
				}
				free_blocks[i] = newblock;
			}
		}

		datablock_t last_block;
		bfs.read_block(target_block.blocks[last_block_num], &last_block);

		int count = 0;
		int offset;
        if (target_block.size > BLOCK_SIZE)
            offset = target_block.size - BLOCK_SIZE;
        else
            offset = target_block.size;
        
		while (data[count] && count < BLOCK_SIZE) {
            last_block.data[count + offset] = data[count];
			count++;
		}
        if (target_block.size > BLOCK_SIZE)
            offset = count - offset;
		// if filled up block but still has more data
		if (strlen(data) - offset > 0) {
			int data_left = strlen(data) - offset;
			bytes_used = (double)data_left / BLOCK_SIZE;
			int blocks_to_allocate = ceil(bytes_used);

			datablock_t data_block[MAX_DATA_BLOCKS];
			for (int i = 0; i < blocks_to_allocate; i++) {
				target_block.blocks[i + last_block_num + 1] = free_blocks[i];

				int count = 0;
				while (data[count + offset] && count < BLOCK_SIZE) {
					data_block[i].data[count] = data[offset + count + i * BLOCK_SIZE];
					count++;
				}
				bfs.write_block(free_blocks[i], (void*)&data_block[i]);
			}
		}
        
		target_block.size += (unsigned int)strlen(data);;
		bfs.write_block(target_block.blocks[last_block_num], (void*)&last_block);
	}
    
	bfs.write_block(target_block_num, (void*)&target_block);
	err_msg = "200 OK";
	sendExit(err_msg, 0, msg);

}

// display the contents of a data file
void FileSys::cat(const char* name)
{
	msg = "";
	dirblock_t curr_dir_block;
	bfs.read_block(curr_dir, &curr_dir_block);

	inode_t target_block;
	short target_block_num;
	bool exists = false;
	bool file = true;

	for (int i = 0; i < curr_dir_block.num_entries; i++) {
		if ((string)curr_dir_block.dir_entries[i].name == (string)name) {
			exists = true;
			if (!isDirectory(curr_dir_block.dir_entries[i].block_num)) {
				target_block_num = curr_dir_block.dir_entries[i].block_num;
				bfs.read_block(target_block_num, &target_block);
				file = true;
			}
			else
				file = false;
		}
	}
	if (!exists) {
		err_msg = "503 File does not exist";
		sendExit(err_msg, 0, msg);
		return;
	}

	if (!file) {
		err_msg = "501 File is a directory";
		sendExit(err_msg, 0, msg);
		return;
	}

	double bytes_read = (double)target_block.size / BLOCK_SIZE;
	int blocks_to_read = ceil(bytes_read);

	datablock_t data_block[MAX_DATA_BLOCKS];
	for (int i = 0; i < blocks_to_read; i++) {
		bfs.read_block(target_block.blocks[i], &data_block[i]);

		int count = 0;
		while (count < BLOCK_SIZE && (count + i * BLOCK_SIZE < target_block.size)) {
            msg += data_block[i].data[count];
			count++;
		}
	}

	err_msg = "200 OK";
	sendExit(err_msg, 1, msg);
}

// display the first N bytes of the file
void FileSys::head(const char* name, unsigned int n)
{
	msg = "";
	dirblock_t curr_dir_block;
	bfs.read_block(curr_dir, &curr_dir_block);

	inode_t target_block;
	short target_block_num;
	bool exists = false;
	bool file = true;

	for (int i = 0; i < curr_dir_block.num_entries; i++) {
		if ((string)curr_dir_block.dir_entries[i].name == (string)name) {
			exists = true;
			if (!isDirectory(curr_dir_block.dir_entries[i].block_num)) {
				target_block_num = curr_dir_block.dir_entries[i].block_num;
				bfs.read_block(target_block_num, &target_block);
				file = true;
			}
			else
				file = false;
		}
	}
	if (!exists) {
		err_msg = "503 File does not exist";
		sendExit(err_msg, 0, msg);
		return;
	}
	if (!file) {
		err_msg = "501 File is a directory";
		sendExit(err_msg, 0, msg);
		return;
	}

	if (n > target_block.size)
		n = target_block.size;
	double bytes_read = (double)n / BLOCK_SIZE;
	int blocks_to_read = ceil(bytes_read);

	datablock_t data_block[MAX_DATA_BLOCKS];
	for (int i = 0; i < blocks_to_read; i++) {
		bfs.read_block(target_block.blocks[i], &data_block[i]);

		int count = 0;
		while (count < BLOCK_SIZE && (count + i * BLOCK_SIZE < n)) {
			msg += data_block[i].data[count];
			count++;
		}
	}

	err_msg = "200 OK";
	sendExit(err_msg, 1, msg);
}

// delete a data file
void FileSys::rm(const char* name)
{
	dirblock_t curr_dir_block;
	bfs.read_block(curr_dir, &curr_dir_block);

	inode_t target_block;
	short target_block_num;
	int index;
	bool exists = false;
	bool file = true;

	for (int i = 0; i < curr_dir_block.num_entries; i++) {
		if ((string)curr_dir_block.dir_entries[i].name == (string)name) {
			exists = true;
			if (!isDirectory(curr_dir_block.dir_entries[i].block_num)) {
				target_block_num = curr_dir_block.dir_entries[i].block_num;
				bfs.read_block(target_block_num, &target_block);
				index = i;
				file = true;
				break;
			}
			else
				file = false;
		}
	}
	if (!exists) {
		err_msg = "503 File does not exist";
		sendExit(err_msg, 0, msg);
		return;
	}
	if (!file) {
		err_msg = "501 File is a directory";
		sendExit(err_msg, 0, msg);
		return;
	}

	int count = 0;
	while (target_block.blocks[count] != 0)
		count++;

	// free blocks
	for (int i = 0; i < count; i++)
		bfs.reclaim_block(target_block.blocks[i]);

	// if want to remove the last entry
	if (target_block_num == curr_dir_block.dir_entries[curr_dir_block.num_entries].block_num) {
		// free block with dir
		bfs.reclaim_block(target_block_num);
	}
	else {
		for (int i = index; i < curr_dir_block.num_entries; i++) {
			curr_dir_block.dir_entries[i] = curr_dir_block.dir_entries[i + 1];
		}
		// free block with dir
		bfs.reclaim_block(target_block_num);

	}
	curr_dir_block.num_entries--;
	// update curr directory
	bfs.write_block(curr_dir, (void*)&curr_dir_block);

	err_msg = "200 OK";
	sendExit(err_msg, 0, msg);
}

// display stats about file or directory
void FileSys::stat(const char* name)
{
	msg = "";
	dirblock_t curr_dir_block;
	bfs.read_block(curr_dir, &curr_dir_block);
	bool exists = false;
	for (int i = 0; i < curr_dir_block.num_entries; i++) {
		if ((string)curr_dir_block.dir_entries[i].name == (string)name) {
			exists = true;
			if (isDirectory(curr_dir_block.dir_entries[i].block_num)) {
				msg += "Directory name: ";
				msg += (string)name;
				msg += "/\n";
				msg += "Directory block: ";
				msg += to_string(curr_dir_block.dir_entries[i].block_num);
			}
			else {
				inode_t target_inode;
				short target_inode_num = curr_dir_block.dir_entries[i].block_num;
				bfs.read_block(target_inode_num, &target_inode);

				int count = 0;
				while (target_inode.blocks[count] != 0) {
					count++;
				}

				msg += "Inode block: ";
				msg += to_string(target_inode_num);
				msg += "\nBytes in file: ";
				msg += to_string(target_inode.size);
				msg += "\nNumber of blocks: ";
				msg += to_string(count + 1);
				msg += "\nFirst block: ";
				msg += to_string(target_inode.blocks[0]);
			}
		}
	}
	if (!exists) {
		err_msg = "503 File does not exist";
		sendExit(err_msg, 0, msg);
		return;
	}

	err_msg = "200 OK";
	sendExit(err_msg, 1, msg);
}

// HELPER FUNCTIONS (optional)

// checks if block is a dirblock
bool FileSys::isDirectory(short block_num) {
	dirblock_t curr_block;
	bfs.read_block(block_num, &curr_block);
	if (curr_block.magic == DIR_MAGIC_NUM)
		return true;
	return false;
}

void FileSys::sendExit(string exit, bool empty, string msg) const {
	const int MAX_SIZE = 7680 + 19;
	char buf[MAX_SIZE];
	string fill = "";

	if (!empty) {
		exit = exit + LINE_END;
		memset(buf, 0, MAX_SIZE);
		strcpy(buf, exit.c_str());

		int bytes_sent = send(fs_sock, buf, MAX_SIZE, 0);
		if (bytes_sent == -1 || bytes_sent == 0) {
			perror("write");
			return;
		}
		exit = "Length:0" + fill + LINE_END;
		memset(buf, 0, MAX_SIZE);
		strcpy(buf, exit.c_str());

		bytes_sent = send(fs_sock, buf, MAX_SIZE, 0);
		if (bytes_sent == -1 || bytes_sent == 0) {
			perror("write");
			return;
		}
		exit = LINE_END;
		memset(buf, 0, MAX_SIZE);
		strcpy(buf, exit.c_str());

		bytes_sent = send(fs_sock, buf, MAX_SIZE, 0);
		if (bytes_sent == -1 || bytes_sent == 0) {
			perror("write");
			return;
		}

	} else {
		exit = exit + LINE_END;
		memset(buf, 0, MAX_SIZE);
		strcpy(buf, exit.c_str());

		int bytes_sent = send(fs_sock, buf, MAX_SIZE, 0);
		if (bytes_sent == -1 || bytes_sent == 0) {
			perror("write");
			return;
		}
		string size = to_string(sizeof(msg));
        
		exit = "Length:" + to_string(msg.length()) + LINE_END;
		memset(buf, 0, MAX_SIZE);
		strcpy(buf, exit.c_str());

		bytes_sent = send(fs_sock, buf, MAX_SIZE, 0);
		if (bytes_sent == -1 || bytes_sent == 0) {
			perror("write");
			return;
		}

		exit = LINE_END;
		memset(buf, 0, MAX_SIZE);
		strcpy(buf, exit.c_str());

		bytes_sent = send(fs_sock, buf, MAX_SIZE, 0);
		if (bytes_sent == -1 || bytes_sent == 0) {
			perror("write");
			return;
		}
		exit = msg;
		memset(buf, 0, MAX_SIZE);
		strcpy(buf, exit.c_str());

		bytes_sent = send(fs_sock, buf, MAX_SIZE, 0);
		if (bytes_sent == -1 || bytes_sent == 0) {
			perror("write");
			return;
		}

	}
}
