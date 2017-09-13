#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_SUB_PATHS 1025
#define MAX_DEPTH 256
#define MAX_NAME_LENGTH 255
#define MAX_PATH_NAME_LENGTH MAX_NAME_LENGTH * MAX_NAME_LENGTH * 2
#define HASH_MAP_DIM MAX_SUB_PATHS * MAX_SUB_PATHS
#define MAX_COMMAND_ARGS 3
#define TANGO_DOWN 1
#define EXIT 0
#define PRIME_KEY_NUMBER 17
#define CREATE_DIR 10
#define CREATE_FILE 11
#define READ_FILE 12
#define WRITE_FILE 13
#define DELETE_RESOURCE 14
#define DELETE_RESOURCE_RECOURSIVLY 15
#define FIND_RESOURCE 16
#define COMMAND_UNKNOWN 17
#define ERRNO_ALLOCATION_FAILED -1
#define ERRNO_FILE_CREATION_FAILED -2
#define ERRNO_DIRECTORY_DOES_NOT_EXIST -3
#define ERRNO_FILE_SYSTEM_STRUCTURE_FAIL -4
#define ERRNO_PATH_LENGTH_EXCEEDS_MAX_LENGTH -5
#define ERRNO_CHAR_NOT_ALLOWED -6
#define ERRNO_COMMAND_UNKNOWN -7
#define BUFFER_DIM 2048
#define TYPE int64_t
#define TYPE_DIM  8
#define TYPE_MASK 7
#define TYPE_N_MASK ~TYPE_MASK
#define TYPE_POW  3

typedef char* String;

struct Data
{
	int lenght;
    String data;
};

struct FileSystem_File
{
	int hash;
	String file_name;
	String metadata;
	String permissions;
	struct Data *raw;
};

struct FileSystem_Folder
{
	int hash;
	String folder_name;
	String metadata;
	String permissions;
	struct FileSytem_Node **direct_path_links;
};
//TODO : improve space complexity, too much redundancy between folder and file data type
struct FileSystem_InnerNode
{
	struct FileSystem_File *file;
	struct FileSystem_Folder *folder;
	String path_name;
	short int isFolder;
};

struct FileSystem_Node
{
	short int children_number;
	short int probe_value;
	struct FileSystem_InnerNode *content;
	struct FileSystem_Node *children[MAX_SUB_PATHS];
};

struct FileSystem_HashMapValue
{
    int path_hash;
    String path_name;
    struct FileSystem_HashMapValue *nextValue;
};

struct FileSystem_HashMap
{
    int resource_hash;
    String resource_name;
    int probe_value;
    struct FileSystem_HashMapValue *last;
    struct FileSystem_HashMapValue *values;
};

typedef struct Data Data;
typedef struct FileSystem_File FileSystem_File;
typedef struct FileSystem_Folder FileSystem_Folder;
typedef struct FileSystem_InnerNode FileSystem_InnerNode;
typedef struct FileSystem_Node FileSystem_Node;
typedef struct FileSystem_HashMap FileSystem_HashMap;
typedef struct FileSystem_HashMapValue FileSystem_HashMapValue;

unsigned long long int generateRawHash(String *parsedInput);

int generateHash(String *parsedInput);

int generateHash_token(String token);

int generateHash_raw(String rawInput);

int generateMapEntryHash(String token);

int generateMapEntryPathHash(String *parsedPath);

inline void vfs_read(String buffer, int length);

inline void vfs_read_i(String buffer, int length, FILE* source);

static inline String vfs_read_s(int length);

String* parseInput(String input);

String* parseInput_ram_optimization(String input);

int initHashMap(FileSystem_HashMap ***hashMap);

int addHashMapEntry(FileSystem_HashMap **hashMap, FileSystem_Node *node, int resource_hash, int path_hash);

int deleteHashMapEntry(FileSystem_HashMap **hashMap, FileSystem_Node *node);

FileSystem_HashMap* getHashMapeEntry(FileSystem_HashMap **hashMap, String resource_name);

int create_file(FileSystem_Node *root, String *parsedPath, String *parsedCommandInput, FileSystem_HashMap **hashMap);

int create_folder(FileSystem_Node *root, String *parsedPath, String *parsedCommandInput, FileSystem_HashMap **hashMap);

Data* read_file(FileSystem_Node *root, String parsedCommandInput);

int write_file(FileSystem_Node *root, String parsedCommandInput, String data);

int delete_resource(FileSystem_Node *root, String parsedCommandInput, FileSystem_HashMap **hashMap);

void delete_sub_resources(FileSystem_Node *node, FileSystem_HashMap **hashMap);

int delete_resource_r(FileSystem_Node *root, String parsedCommandInput, FileSystem_HashMap **hashMap);

FileSystem_HashMap* find_resource(FileSystem_HashMap** hashMap, String token);

void merge_sort(String *paths,int p,int r);

int comp(String L,String R);

void merge(String *arr,int l,int m,int r);

inline int is_empty(FileSystem_Node **buf, size_t size);

static int sort_strcmp (const void * a, const void * b);

void sort(const String *arr, int n);

String fetchIOCommandInput();

int decodeIOCommandInput(String commandInput, String **parsedIOCommands, String **parsedPath, String *commandMemorySpace);

int executeIOCommandInput(FileSystem_Node *root, FileSystem_HashMap **hashMap, int operation_code, String *parsedPath, String *parsedCommandInput, String extra);

int io_operationsLoop(FileSystem_Node *root, FileSystem_HashMap **hashMap);

int vf_vf_pow(int base, int exp);

String relativePath;

int main()
{
    freopen("small_pruning.in", "r", stdin);
	FileSystem_Node *root = (FileSystem_Node*)calloc(1, sizeof(FileSystem_Node));
	root->children_number = 0;
	root->content = (FileSystem_InnerNode*) malloc(sizeof(FileSystem_InnerNode));
	root->content->isFolder = 1;
	FileSystem_HashMap **hashMap;
    initHashMap(&hashMap);
    io_operationsLoop(root, hashMap);
    free(hashMap);
    free(root);
	return EXIT;
}

int my_strcmp(char *str1,char *str2,int len1,int len2) {
    if(len1 != len2){
        return 0;
    }

    TYPE a,b;
    len1 >>= TYPE_POW;

    for(;len1--;){
        a = *(TYPE *) str1;
        str1 += TYPE_DIM;
        b = *(TYPE *) str2;
        str2 += TYPE_DIM;
        if (a == NULL) {
            if(b == NULL){
                return 1;
            }
            else{
                return 0;
            }
        }
    };

    str1 -= TYPE_DIM;
    str2 -= TYPE_DIM;

    int s1;
    int s2;
    do {
        s1 = *str1++;
        s2 = *str2++;
        if (s1 == 0)
            break;
    } while (s1 == s2);

    return s1 == s2;
}

void my_strcpy(char *dest,char *src,unsigned int src_len){

    int i = (src_len & TYPE_N_MASK)>>TYPE_POW;
    for(;i--;){
        *(TYPE*)dest = *(TYPE*)src;
        dest += TYPE_DIM;
        src  += TYPE_DIM;
    }

    switch(src_len & TYPE_MASK){
        case 7:
            *dest++ = *src++;
        case 6:
            *dest++ = *src++;
        case 5:
            *dest++ = *src++;
        case 4:
            *dest++ = *src++;
        case 3:
            *dest++ = *src++;
        case 2:
            *dest++ = *src++;
        case 1:
            *dest++ = *src;
        default:
            *dest = 0;
            break;
    }
}

int vf_pow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}

size_t hashf(register char *string) {
    /*register char temp = *stringa;
    register int hash = temp << 7;
    register unsigned i = 1;
    while (temp) {
        hash = (1000003 * hash) ^ temp;
        temp = *(stringa++);
        i++;
    }
    hash ^= i;
    return (size_t) hash;*/
    unsigned int hash = 5381;
    int c;
    while(c = *string++)
        hash = ((hash << 5) + hash) + c;
    return (size_t) hash;
}


unsigned long long int generateRawHash(String *parsedInput)
{
	int token = 0;
	int index = 0;
	register unsigned long long int tempValue = 0;
    int length = strlen(parsedInput[token]);

	while (token < MAX_DEPTH && parsedInput[token] != NULL)
	{
		/*while (index < length && parsedInput[token][index] != '\0' && parsedInput[token][index] != '\n')
		{
                tempValue += parsedInput[token][index] * vf_pow(PRIME_KEY_NUMBER, index + 1);
                index++;
        }*/
        tempValue += hashf(parsedInput[token]);
		token++;
		if(parsedInput[token] != NULL)
            length = strlen(parsedInput[token]);
		index = 0;
	}

	return tempValue;
}

int generateHash(String *parsedInput)
{
	return generateRawHash(parsedInput) % MAX_SUB_PATHS;
}

int generateHash_token(String token)
{
	int index = 0;
	unsigned long long int tempValue = 0;
	int length = strlen(token);
	/*while (index < length && token[index] != '\0' && token[index] != '\n')
	{
		tempValue += token[index] * vf_pow(PRIME_KEY_NUMBER, index + 1);
		index++;
    }*/
    tempValue = hashf(token);
	return tempValue % MAX_SUB_PATHS;
}

/*int generateHash_raw(String rawInput)
{
	String *parsedInput = parseInput(rawInput);
	return generateHash(parsedInput);
}*/

int generateMapEntryHash(String token)
{
    int index = 0;
	unsigned long long int tempValue = 0;
	int length = strlen(token);
	/*while (index < length && token[index] != '\0' && token[index] != '\n')
	{
		tempValue += token[index] * vf_pow(PRIME_KEY_NUMBER, index + 1);
		index++;
    }*/
    tempValue = hashf(token);
	return tempValue % (MAX_SUB_PATHS * MAX_SUB_PATHS);
}

int generateMapEntryPathHash(String *parsedPath)
{
	return generateRawHash(parsedPath) % (MAX_SUB_PATHS * MAX_SUB_PATHS);
}

int initHashMap(FileSystem_HashMap ***hashMap)
{
    *hashMap = (FileSystem_HashMap**) calloc(HASH_MAP_DIM, sizeof(FileSystem_HashMap*));
    if(*hashMap == NULL)
        return ERRNO_ALLOCATION_FAILED;
    return TANGO_DOWN;
}

int addHashMapEntry(FileSystem_HashMap **hashMap, FileSystem_Node *node, int resource_hash, int path_hash)
{
    if(hashMap == NULL)
        return ERRNO_FILE_SYSTEM_STRUCTURE_FAIL;
    String resource_name = NULL;
    if(node->content->file != NULL)
        resource_name = node->content->file->file_name;
    else
        resource_name = node->content->folder->folder_name;
    int map_entry_hash = generateMapEntryHash(resource_name), length;
    if(hashMap[map_entry_hash] == NULL)
    {
        hashMap[map_entry_hash] = (FileSystem_HashMap*) malloc(sizeof(FileSystem_HashMap));
        if(hashMap[map_entry_hash] == NULL)
            return ERRNO_ALLOCATION_FAILED;
        hashMap[map_entry_hash]->resource_hash = resource_hash;
        hashMap[map_entry_hash]->probe_value = -1;
        length = strlen(resource_name);
        hashMap[map_entry_hash]->resource_name = (String) malloc(sizeof(char) * (length + 1));
        if(hashMap[map_entry_hash]->resource_name == NULL)
            return ERRNO_ALLOCATION_FAILED;
        my_strcpy(hashMap[map_entry_hash]->resource_name, resource_name, length);
        hashMap[map_entry_hash]->values = NULL;
        hashMap[map_entry_hash]->last = NULL;
    }
    else if(strcmp(hashMap[map_entry_hash]->resource_name, resource_name) != 0)
    {
        int offset = 1, backup_map_entry_hash = map_entry_hash;
        while((map_entry_hash + offset) < HASH_MAP_DIM && hashMap[map_entry_hash + offset++] != NULL);
        if((map_entry_hash + offset) < HASH_MAP_DIM)
            map_entry_hash = map_entry_hash + offset - 1;
        else
        {
            offset = 0;
            while(offset < map_entry_hash && hashMap[offset++] != NULL);
            if(offset >= map_entry_hash)
                return ERRNO_ALLOCATION_FAILED;
            map_entry_hash = offset;
        }
        hashMap[backup_map_entry_hash]->probe_value = map_entry_hash;
        hashMap[map_entry_hash] = (FileSystem_HashMap*) malloc(sizeof(FileSystem_HashMap));
        if(hashMap[map_entry_hash] == NULL)
            return ERRNO_ALLOCATION_FAILED;
        hashMap[map_entry_hash]->resource_hash = resource_hash;
        hashMap[map_entry_hash]->probe_value = -1;
        length = strlen(resource_name);
        hashMap[map_entry_hash]->resource_name = (String) malloc(sizeof(char) * (length + 1));
        if(hashMap[map_entry_hash]->resource_name == NULL)
            return ERRNO_ALLOCATION_FAILED;
        my_strcpy(hashMap[map_entry_hash]->resource_name, resource_name, length);
        hashMap[map_entry_hash]->values = NULL;
    }
    FileSystem_HashMap *query = hashMap[map_entry_hash];
    FileSystem_HashMapValue *value = query->values;
    FileSystem_HashMapValue* temp = (FileSystem_HashMapValue *) malloc(sizeof(FileSystem_HashMapValue));
    if(temp == NULL)
        return ERRNO_ALLOCATION_FAILED;
    temp->path_hash = path_hash;
    length = strlen(node->content->path_name);
    temp->path_name = (String) malloc(sizeof(char) * (length + 1));
    if(temp->path_name == NULL)
        return ERRNO_ALLOCATION_FAILED;
    my_strcpy(temp->path_name, node->content->path_name, length);
    temp->nextValue = value;
    query->values = temp;
    return TANGO_DOWN;
}

int deleteHashMapEntry(FileSystem_HashMap **hashMap, FileSystem_Node *node)
{
    if(hashMap == NULL)
        return ERRNO_FILE_SYSTEM_STRUCTURE_FAIL;
    String resource_name = NULL;
    if(node->content->file != NULL)
        resource_name = node->content->file->file_name;
    else
        resource_name = node->content->folder->folder_name;
    int map_entry_hash = generateMapEntryHash(resource_name);
    if(hashMap[map_entry_hash] == NULL)
        return ERRNO_DIRECTORY_DOES_NOT_EXIST;
    FileSystem_HashMap *query = hashMap[map_entry_hash];
    while(strcmp(resource_name, query->resource_name) != 0)
        query = hashMap[map_entry_hash = query->probe_value];
    FileSystem_HashMapValue *value = query->values, *prev = NULL;
    int counter = 0;
    while(value != NULL)
    {
        if(strcmp(node->content->path_name, value->path_name) == 0)
        {
            if(prev != NULL)
                prev->nextValue = value->nextValue;
            else if(value->nextValue != NULL)
                query->values = value->nextValue;
            else
            {
                query->values = NULL;
                free(hashMap[map_entry_hash]);
                hashMap[map_entry_hash] = NULL;
            }
            free(value);
        }
        counter++;
        prev = value;
        value = value->nextValue;
    }

    return ERRNO_DIRECTORY_DOES_NOT_EXIST;
}

FileSystem_HashMap* getHashMapeEntry(FileSystem_HashMap **hashMap, String resource_name)
{
    if(hashMap == NULL)
        return NULL;
    FileSystem_HashMap* resource = hashMap[generateMapEntryHash(resource_name)];
    while(resource != NULL && strcmp(resource->resource_name, resource_name) != 0)
        resource = hashMap[resource->probe_value];
    return resource;
}

int create_file(FileSystem_Node *root, String *parsedPath, String *parsedCommandInput, FileSystem_HashMap **hashMap)
{
	if (root == NULL || root->children == NULL)
		return ERRNO_FILE_SYSTEM_STRUCTURE_FAIL;
    int token = 0, hash, index = 1, relative_index = 0, end_path = 0;
	FileSystem_Node *node = root, **probedChildren;
	while (token < MAX_DEPTH && !end_path)
	{
        while(parsedCommandInput[1][index] != '/' && parsedCommandInput[1][index] != '\0')
            relativePath[relative_index++] = parsedCommandInput[1][index++];
        if(parsedCommandInput[1][index] == '\0')
            end_path = 1;
        relativePath[relative_index] = '\0';
        index++;
        relative_index = 0;
		hash = generateHash_token(relativePath);
		if (end_path)
		{
                if(node != NULL && node->content->isFolder && node->children[hash] == NULL)
                {
                    node->children[hash] = (FileSystem_Node*)calloc(1, sizeof(FileSystem_Node));
                    if (node->children[hash] == NULL)
                        return ERRNO_ALLOCATION_FAILED;
                }
                else if(node != NULL && node->content->isFolder && ((node->children[hash]->content->file != NULL &&
                        strcmp(node->children[hash]->content->file->file_name, relativePath) != 0) ||
                        (node->children[hash]->content->folder != NULL &&
                        strcmp(node->children[hash]->content->folder->folder_name, relativePath) != 0)))
                {
                    int offset = 1, hash_backup = hash;
                    while((hash + offset) < MAX_SUB_PATHS && node->children[hash + offset++] != NULL);
                    if((hash + offset) < MAX_SUB_PATHS)
                        hash = hash + offset - 1;
                    else
                    {
                        offset = 0;
                        while(offset < hash && node->children[offset++] != NULL);
                        if(offset >= hash)
                            return ERRNO_ALLOCATION_FAILED;
                        hash = offset;
                    }
                    if(node == root)
                        node = node->children[hash_backup];
                    node->probe_value = hash;
                    node->children[hash] = (FileSystem_Node*) calloc(1, sizeof(FileSystem_Node));
                    if (node->children[hash] == NULL)
                        return ERRNO_ALLOCATION_FAILED;
                }
                else
                    return ERRNO_ALLOCATION_FAILED;
                node->children_number++;
				node = node->children[hash];
				node->children_number = 0;
				node->probe_value = -1;
				node->content = (FileSystem_InnerNode*)malloc(sizeof(FileSystem_InnerNode));
				if(node->content == NULL)
                    return ERRNO_ALLOCATION_FAILED;
                int length = strlen(parsedCommandInput[1]);
                node->content->path_name = (String) malloc(sizeof(char) * (length +1));
                if(node->content->path_name == NULL)
                    return ERRNO_ALLOCATION_FAILED;
                my_strcpy(node->content->path_name, parsedCommandInput[1], length);
				node->content->isFolder = 0;
				node->content->folder = NULL;
				node->content->file = (FileSystem_File*)malloc(sizeof(FileSystem_File));
				if (node->content->file == NULL)
					return ERRNO_ALLOCATION_FAILED;
                length = strlen(relativePath);
				node->content->file->file_name = (String)malloc(sizeof(char) * (length + 1));
				if (node->content->file->file_name == NULL)
					return ERRNO_ALLOCATION_FAILED;
				my_strcpy(node->content->file->file_name, relativePath, length);
				node->content->file->raw = NULL;
				node->content->file->hash = generateMapEntryHash(parsedCommandInput[1]);
				addHashMapEntry(hashMap, node, hash, node->content->file->hash);
				return TANGO_DOWN;
		}
		else if (node == NULL)
				return ERRNO_DIRECTORY_DOES_NOT_EXIST;
        if(node->children[hash] != NULL)
            node = node->children[hash];
        else
            return ERRNO_DIRECTORY_DOES_NOT_EXIST;
        if(node != NULL && node != root && node->content->isFolder && strcmp(relativePath, node->content->folder->folder_name) != 0)
        {
            probedChildren = node->children;
            while(node != NULL && node->content->folder != NULL && strcmp(relativePath, node->content->folder->folder_name) != 0)
            {
                if(node->probe_value == -1)
                    return ERRNO_DIRECTORY_DOES_NOT_EXIST;
                node = probedChildren[node->probe_value];
            }
        }
		token++;
	}
	return ERRNO_PATH_LENGTH_EXCEEDS_MAX_LENGTH;
}

int create_folder(FileSystem_Node *root, String *parsedPath, String *parsedCommandInput, FileSystem_HashMap **hashMap)
{
	if (root == NULL)
		return ERRNO_FILE_SYSTEM_STRUCTURE_FAIL;
    int token = 0, hash, index = 1, relative_index = 0, end_path = 0;
	FileSystem_Node *node = root, **probedChildren;
	while (token < MAX_DEPTH && !end_path)
	{
		while(parsedCommandInput[1][index] != '/' && parsedCommandInput[1][index] != '\0')
            relativePath[relative_index++] = parsedCommandInput[1][index++];
        if(parsedCommandInput[1][index] == '\0')
            end_path = 1;
        relativePath[relative_index] = '\0';
        index++;
        relative_index = 0;
		hash = generateHash_token(relativePath);
		if (end_path)
		{
            if(node != NULL && node->content->isFolder && node->children[hash] == NULL)
            {
                node->children[hash] = (FileSystem_Node*)calloc(1, sizeof(FileSystem_Node));
                if (node->children[hash] == NULL)
                    return ERRNO_ALLOCATION_FAILED;
            }
            else if(node != NULL && node->content->isFolder && ((node->children[hash]->content->file != NULL &&
                        strcmp(node->children[hash]->content->file->file_name, relativePath) != 0) ||
                        (node->children[hash]->content->folder != NULL &&
                        strcmp(node->children[hash]->content->folder->folder_name, relativePath) != 0)))
            {
                int offset = 1, hash_backup = hash;
                while((hash + offset) < MAX_SUB_PATHS && node->children[hash + offset++] != NULL);
                if((hash + offset) < MAX_SUB_PATHS)
                    hash = hash + offset - 1;
                else
                {
                    offset = 0;
                    while(offset < hash && node->children[offset++] != NULL);
                    if(offset >= hash)
                        return ERRNO_ALLOCATION_FAILED;
                    hash = offset;
                }
                if(node == root)
                    node = node->children[hash_backup];
                node->probe_value = hash;
                node->children[hash] = (FileSystem_Node*) calloc(1, sizeof(FileSystem_Node));
                if (node->children[hash] == NULL)
                    return ERRNO_ALLOCATION_FAILED;
            }
            else
                return ERRNO_ALLOCATION_FAILED;
            node->children_number++;
			node = node->children[hash];
			node->children_number = 0;
			node->probe_value = -1;
			node->content = (FileSystem_InnerNode*)malloc(sizeof(FileSystem_InnerNode));
			if(node->content == NULL)
                    return ERRNO_ALLOCATION_FAILED;
            int length = strlen(parsedCommandInput[1]);
			node->content->path_name = (String) malloc(sizeof(char) * (length + 1));
            if(node->content->path_name == NULL)
                return ERRNO_ALLOCATION_FAILED;
            my_strcpy(node->content->path_name, parsedCommandInput[1], length);
			node->content->isFolder = 1;
			node->content->file = NULL;
			node->content->folder = (FileSystem_Folder*)malloc(sizeof(FileSystem_Folder));
			if (node->content->folder == NULL)
                return ERRNO_ALLOCATION_FAILED;
            length = strlen(relativePath);
			node->content->folder->folder_name = (String)malloc(sizeof(char) * (length + 1));
			if (node->content->folder->folder_name == NULL)
                return ERRNO_ALLOCATION_FAILED;
			my_strcpy(node->content->folder->folder_name, relativePath, length);
            node->content->folder->hash = generateMapEntryHash(parsedCommandInput[1]);
			addHashMapEntry(hashMap, node, hash, node->content->folder->hash);
		}
		else if(node != NULL)
		{
            node = node->children[hash];
			if(node != NULL && node != root && node->content->isFolder && strcmp(relativePath, node->content->folder->folder_name) != 0)
            {
                probedChildren = node->children;
                while(node != NULL && node->content->folder != NULL && strcmp(relativePath, node->content->folder->folder_name) != 0)
                {
                    if(node->probe_value == -1)
                        return ERRNO_DIRECTORY_DOES_NOT_EXIST;
                    node = probedChildren[node->probe_value];
                }
            }
            else if(node == NULL)
                return ERRNO_DIRECTORY_DOES_NOT_EXIST;
        }
		token++;
	}
	return token < MAX_DEPTH ? TANGO_DOWN : ERRNO_PATH_LENGTH_EXCEEDS_MAX_LENGTH;
}

Data* read_file(FileSystem_Node *root, String parsedCommandInput)
{
	if (root == NULL || root->children == NULL)
		return (void *)ERRNO_FILE_SYSTEM_STRUCTURE_FAIL;
    int token = 0, hash, index = 1, relative_index = 0, end_path = 0;
	FileSystem_Node *node = root, **probedChildren;
	String tmp_str;
	int file_hash = generateMapEntryHash(parsedCommandInput);
	while (token < MAX_DEPTH && !end_path)
	{
        while(parsedCommandInput[index] != '/' && parsedCommandInput[index] != '\0')
            relativePath[relative_index++] = parsedCommandInput[index++];
        if(parsedCommandInput[index] == '\0')
            end_path = 1;
        relativePath[relative_index] = '\0';
        index++;
        relative_index = 0;
		hash = generateHash_token(relativePath);
		if (node->children[hash] == NULL)
			return (void *)ERRNO_DIRECTORY_DOES_NOT_EXIST;
        probedChildren = node->children;
		node = node->children[hash];
		if(!node->content->isFolder)
            tmp_str = node->content->file->file_name;
        else
            tmp_str = node->content->folder->folder_name;
        while(strcmp(relativePath, tmp_str) != 0)
        {
            if(node->probe_value == -1)
                return (void *)ERRNO_DIRECTORY_DOES_NOT_EXIST;
            node = probedChildren[node->probe_value];
            if(node->content->file != NULL)
                tmp_str = node->content->file->file_name;
            else
                tmp_str = node->content->folder->folder_name;
        }
		if(node->content->file != NULL && node->content->file->hash == file_hash)
			return node->content->file->raw;
		token++;
	}
	return (void *)ERRNO_PATH_LENGTH_EXCEEDS_MAX_LENGTH;
}

int write_file(FileSystem_Node *root, String parsedCommandInput, String data)
{
	if (root == NULL || root->children == NULL)
		return ERRNO_FILE_SYSTEM_STRUCTURE_FAIL;
    int token = 0, hash, index = 1, relative_index = 0, end_path = 0;
	FileSystem_Node *node = root, **probedChildren;
	int file_hash = generateMapEntryHash(parsedCommandInput);
	String tmp_str;
	while (token < MAX_DEPTH && !end_path)
	{
        while(parsedCommandInput[index] != '/' && parsedCommandInput[index] != '\0')
            relativePath[relative_index++] = parsedCommandInput[index++];
        if(parsedCommandInput[index] == '\0')
            end_path = 1;
        relativePath[relative_index] = '\0';
        index++;
        relative_index = 0;
		hash = generateHash_token(relativePath);
		if (node->children[hash] == NULL)
			return ERRNO_DIRECTORY_DOES_NOT_EXIST;
        probedChildren = node->children;
        node = node->children[hash];
        if(node->content == NULL)
            return ERRNO_ALLOCATION_FAILED;
		if(!node->content->isFolder)
            tmp_str = node->content->file->file_name;
        else
            tmp_str = node->content->folder->folder_name;
        while(strcmp(relativePath, tmp_str) != 0)
        {
            if(node->probe_value == -1)
                return ERRNO_DIRECTORY_DOES_NOT_EXIST;
            node = probedChildren[node->probe_value];
            if(node->content->file != NULL)
                tmp_str = node->content->file->file_name;
            else
                tmp_str = node->content->folder->folder_name;
        }
		if(relativePath != NULL && node->content->file != NULL && node->content->file->hash == file_hash)
		{
            int length = strlen(data);
			if(node->content->file->raw == NULL)
			{
				node->content->file->raw = (Data*) malloc(sizeof(Data));
				if (node->content->file->raw == NULL)
					return ERRNO_ALLOCATION_FAILED;
				node->content->file->raw->data = (String) malloc(sizeof(char) * (length + 1));
				if(node->content->file->raw->data == NULL)
					return ERRNO_ALLOCATION_FAILED;
			}
			else
                node->content->file->raw = (String) realloc(node->content->file->raw, sizeof(char) * (length + 1));
			my_strcpy(node->content->file->raw->data, data, length);
			node->content->file->raw->lenght = length;
			return TANGO_DOWN;
		}
		token++;
	}
	return ERRNO_PATH_LENGTH_EXCEEDS_MAX_LENGTH;
}

int delete_resource(FileSystem_Node *root, String parsedCommandInput, FileSystem_HashMap **hashMap)
{
	if (root == NULL || root->children == NULL)
		return ERRNO_FILE_SYSTEM_STRUCTURE_FAIL;
    int token = 0, index = 1, relative_index = 0, end_path = 0;
	FileSystem_Node *node = root, **probedChildren;
	FileSystem_Node *upper_node = NULL;
	int file_hash = generateMapEntryHash(parsedCommandInput), hash, probed_index = -1;
	String tmp_str;
	while (token < MAX_DEPTH && !end_path)
	{
        while(parsedCommandInput[index] != '/' && parsedCommandInput[index] != '\0')
            relativePath[relative_index++] = parsedCommandInput[index++];
        if(parsedCommandInput[index] == '\0')
            end_path = 1;
        relativePath[relative_index] = '\0';
        index++;
        relative_index = 0;
		hash = generateHash_token(relativePath);
		if (node->children[hash] == NULL)
			return ERRNO_DIRECTORY_DOES_NOT_EXIST;
        upper_node = node;
        probedChildren = node->children;
        node = node->children[hash];
		if(node->content->file != NULL)
            tmp_str = node->content->file->file_name;
        else
            tmp_str = node->content->folder->folder_name;
        while(strcmp(relativePath, tmp_str) != 0)
        {
            probed_index = node->probe_value;
            if(probed_index == -1)
                return ERRNO_DIRECTORY_DOES_NOT_EXIST;
            node = probedChildren[node->probe_value];
            if(node->content->file != NULL)
                tmp_str = node->content->file->file_name;
            else
                tmp_str = node->content->folder->folder_name;
        }
        if(probed_index != -1)
            hash = probed_index;
		if(strcmp(tmp_str, relativePath) == 0 && end_path)
            {
                if(!node->content->isFolder && node->content->file != NULL)
                {
                    deleteHashMapEntry(hashMap, node);
                    free(node->content->file->raw);
                    free(node->content->file->file_name);
                    free(node->content->file);
                    free(node->content->path_name);
                    free(node->content);
                    upper_node->children_number--;
                    upper_node->children[hash] = NULL;

                    if(probed_index != -1 && node->probe_value > 0)
                    {

                        upper_node->children[hash] = upper_node->children[node->probe_value];
                        upper_node->children[node->probe_value] = NULL;
                    }
                    free(node);
                    return TANGO_DOWN;
                }
                else if(node->content->isFolder && node->content->folder != NULL && node->children_number == 0)
                {
                    deleteHashMapEntry(hashMap, node);
                    free(node->content->folder->folder_name);
                    free(node->content->folder);
                    free(node->content->path_name);
                    free(node->content);
                    upper_node->children_number--;
                    upper_node->children[hash] = NULL;
                    if(probed_index != -1 && node->probe_value > 0)
                    {
                        upper_node->children[hash] = upper_node->children[node->probe_value];
                        upper_node->children[node->probe_value] = NULL;
                    }
                    free(node);
                    return TANGO_DOWN;
                }
                else return ERRNO_FILE_SYSTEM_STRUCTURE_FAIL;
                upper_node->children[hash] = NULL;
            }
		token++;
	}
	return ERRNO_DIRECTORY_DOES_NOT_EXIST;
}

void delete_sub_resources(FileSystem_Node *node, FileSystem_HashMap **hashMap)
{
    int sub_resources_index = 0;
    for(; sub_resources_index < MAX_SUB_PATHS; sub_resources_index++)  // ***WARNING*** NOT EFFICIENT --- > USE INNERNODES DIRECT LINKS INSTEAD
        if(node->content != NULL && node->content->file == NULL && node->children[sub_resources_index] != NULL)
        {
            node->children_number--;
            delete_sub_resources(node->children[sub_resources_index], hashMap);
        }
    if(node->content->file != NULL)
    {
        deleteHashMapEntry(hashMap, node);
        free(node->content->file->file_name);
        if(node->content->file->raw != NULL)
            free(node->content->file->raw);
        free(node->content->path_name);
        free(node->content->file);
        free(node->content);
        free(node);
    }
    else
    {
        deleteHashMapEntry(hashMap, node);
        free(node->content->folder->folder_name);
        free(node->content->path_name);
        free(node->content->folder);
        free(node->content);
        free(node);
    }
}

int delete_resource_r(FileSystem_Node *root, String parsedCommandInput, FileSystem_HashMap **hashMap)
{
	if (root == NULL || root->children == NULL)
		return ERRNO_FILE_SYSTEM_STRUCTURE_FAIL;
    int token = 0, index = 1, relative_index = 0, end_path = 0;
	FileSystem_Node *node = root, **probedChildren;
	FileSystem_Node *upper_node = NULL;
	int file_hash = generateMapEntryHash(parsedCommandInput), hash, probed_index = -1;
	String tmp_str;
	while (token < MAX_DEPTH && !end_path)
	{
        while(parsedCommandInput[index] != '/' && parsedCommandInput[index] != '\0')
            relativePath[relative_index++] = parsedCommandInput[index++];
        if(parsedCommandInput[index] == '\0')
            end_path = 1;
        relativePath[relative_index] = '\0';
        index++;
        relative_index = 0;
		hash = generateHash_token(relativePath);
		if (node->children[hash] == NULL)
			return ERRNO_DIRECTORY_DOES_NOT_EXIST;
        upper_node = node;
        probedChildren = node->children;
		node = node->children[hash];
		if(node->content->file != NULL)
            tmp_str = node->content->file->file_name;
        else
            tmp_str = node->content->folder->folder_name;
        while(strcmp(relativePath, tmp_str) != 0)
        {
            probed_index = node->probe_value;
            if(probed_index == -1)
                return ERRNO_DIRECTORY_DOES_NOT_EXIST;
            node = probedChildren[node->probe_value];
            if(node->content->file != NULL)
                tmp_str = node->content->file->file_name;
            else
                tmp_str = node->content->folder->folder_name;
        }
        if(probed_index != -1)
            hash = probed_index;
		if(strcmp(tmp_str, relativePath) == 0 && end_path)
            {
                if(!node->content->isFolder && node->content->file != NULL)
                {
                    deleteHashMapEntry(hashMap, node);
                    if(node->content->file->raw != NULL)
                        free(node->content->file->raw);
                    free(node->content->file->file_name);
                    free(node->content->file);
                    free(node->content->path_name);
                    free(node->content);
                    upper_node->children_number--;
                    upper_node->children[hash] = NULL;
                    if(probed_index != -1 && node->probe_value > 0)
                    {

                        upper_node->children[hash] = upper_node->children[node->probe_value];
                        upper_node->children[node->probe_value] = NULL;
                    }
                    free(node);
                    return TANGO_DOWN;
                }
                else if(node->content->isFolder && node->content->folder != NULL)
                {
                    short int probe_value = node->probe_value;
                    delete_sub_resources(node, hashMap);
                    upper_node->children_number--;
                    upper_node->children[hash] = NULL;
                    if(probed_index != -1 && node->probe_value > 0)
                    {
                        upper_node->children[hash] = upper_node->children[probe_value];
                        upper_node->children[probe_value] = NULL;
                    }
                    return TANGO_DOWN;
                }
            }
		token++;
	}
	return ERRNO_DIRECTORY_DOES_NOT_EXIST;
}

FileSystem_HashMap* find_resource(FileSystem_HashMap** hashMap, String token)
{
    return getHashMapeEntry(hashMap, token);
}

void merge_sort(String *paths,int p,int r){
    if(p<r){
        int q = (p+r)/2;
        merge_sort(paths,p,q);
        merge_sort(paths,q+1,r);
        merge(paths,p,q,r);
    }
}

int comp(String L,String R){
    if(L == NULL && R == NULL)
        return 0;
    if(L == NULL && R != NULL)
        return 1;
    if(L != NULL && R == NULL)
        return -1;

    return strcmp(L,R);
}

void merge(String *arr,int l,int m,int r){
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;
    String L[n1], R[n2];
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];
    i = 0; j = 0; k = 0;
    while (i < n1 && j < n2)
    {
        if(comp(L[i],R[j]) <= 0)
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}

static inline char vfs_getchar()
{
    static char buffer[2048];
    const String msc_little_indian = &(buffer[2048]);
    static String pointer = &(buffer[2048]);

    if(pointer == msc_little_indian)
    {
        fread(buffer, sizeof(char), 2048, stdin);
        pointer = buffer;
    }
    return *pointer++;
    //return (char) getchar();
}

inline void vfs_read(String buffer, int length)
{
	fgets(buffer, length, stdin);
}

inline void vfs_read_i(String buffer, int length, FILE* source)
{
	fgets(buffer, length, source);
}

static inline String vfs_read_s(int length)
{
    static String buffer = NULL;
    if(buffer == NULL)
        buffer = (String) malloc(sizeof(char) * length);
    char temp;
    int i = 0, dataFlag = 0;
    do
    {
        buffer[i++] = vfs_getchar();
        if(buffer[i - 1] == ' ' && !dataFlag)
        {
            while((temp = vfs_getchar()) == ' ');
            buffer[i++] = temp;
            if(temp == '"')
                dataFlag = 1;
        }
        if(buffer[i - 1] == '"')
            dataFlag = 1;
    }
    while(buffer[i - 1] != '\n' && buffer[i - 1] != '\0');
    buffer[i] = '\0';
    return buffer;
}

/*String* parseInput(String input)
{
	int index = 0, depth = 0, char_counter = 0; //first char is root folder
	while(input[index] != '\0' && input[index] != '\n')
	{
        if(path[depth] == NULL)
            path[depth] = (String) malloc(sizeof(char) * MAX_NAME_LENGTH);
		index++;
		do
		{
			path[depth][char_counter++] = input[index++];
			if(!isalpha(input[index - 1]) && !isdigit(input[index - 1]))
                return NULL;
        }
		while(input[index] != '/' && input[index] != '\0' && input[index] != '\n');
		path[depth][char_counter] = '\0';
		depth++;
		char_counter = 0;
	}
	path[depth] = NULL;
	return path;
}

String* parseInput_ram_optimization(String input)
{
	int index = 0, depth = 0, char_counter = 0; //first char is root folder
	String *path = (String*) malloc(sizeof(String) * MAX_SUB_PATHS);
	String temp = (String) malloc(sizeof(char) * MAX_NAME_LENGTH);
	while(input[index] != '\0' && input[index] != '\n')
	{
		index++;
		do
		{
			temp[char_counter++] = input[index++];
            if(!isalnum(input[index - 1]))
                return NULL;
        }
		while(input[index] != '/' && input[index] != '\0' && input[index] != '\n');
		temp[char_counter] = '\0';
        path[depth] = (String) malloc(sizeof(char) * char_counter); //allocate just what's needed
        my_strcpy(path[depth], temp); //slow down execution (a bit, adding linear O(char_counter) complexity)
		depth++;
		char_counter = 0;
	}
	path[depth] = NULL;
	return path;
}*/

inline int is_empty(FileSystem_Node **buf, size_t size)
{
    return buf[0] == NULL && !memcmp(*buf, *buf + 1, size - 1);
}

static int sort_strcmp (const void * a, const void * b)
{
    return strcmp (*(const String*) a, *(const String*) b);
}

void sort(const String *arr, int n)
{
    qsort ((void *)arr, n, sizeof (const String), sort_strcmp);
}

String fetchIOCommandInput()
{
    return vfs_read_s(MAX_PATH_NAME_LENGTH * 2);
}

int decodeIOCommandInput(String commandInput, String **parsedIOCommands, String **parsedPath, String *commandMemorySpace)
{
    //if(*parsedIOCommands != NULL)
    //    free(*parsedIOCommands);
    /*static String *tempParsed = NULL;
    if(tempParsed == NULL)
        tempParsed = (String*) calloc(MAX_COMMAND_ARGS, sizeof(String));*/
    //String *tempParsed = (String*) calloc(MAX_COMMAND_ARGS, sizeof(String));
    //String tempParsed[MAX_COMMAND_ARGS];
    int index = 0, command_token = 0, command_token_index = 0, data_flag = 0, length = 0;
    length = strlen(commandInput);
    while(commandInput[index] != '\0' && commandInput[index] != '\n')
    {
        if(commandMemorySpace[command_token] == NULL)
            commandMemorySpace[command_token] = (String) malloc(sizeof(char) * MAX_PATH_NAME_LENGTH);
        while(index < length && (data_flag || commandInput[index] != ' ') && commandInput[index] != '\n' && commandInput[index] != '\0' && commandInput[index] != '"')
            commandMemorySpace[command_token][command_token_index++] = commandInput[index++];
        if(commandInput[index] == ' ')
            index++;
        commandMemorySpace[command_token][command_token_index] = '\0';
        command_token++;
        command_token_index = 0;
        if(commandInput[index] == '"')
        {
            data_flag = 1;
            index++;
        }
    }
    while(command_token < MAX_COMMAND_ARGS)
        commandMemorySpace[command_token++] = NULL;
    /*if(commandMemorySpace[1] != NULL)
    {
        *parsedPath = parseInput(commandMemorySpace[1]);
        if(*parsedPath == NULL)
            return ERRNO_CHAR_NOT_ALLOWED;
    }*/
    if(commandInput[0] == '\n')
        return COMMAND_UNKNOWN;
    *parsedIOCommands = commandMemorySpace;
    if(strncmp(commandMemorySpace[0], "create_dir", 10) == 0)
        return CREATE_DIR;
    if(strncmp(commandMemorySpace[0], "create", 6) == 0)
        return CREATE_FILE;
    if(strncmp(commandMemorySpace[0], "read", 4) == 0)
        return READ_FILE;
    if(strncmp(commandMemorySpace[0], "write", 5) == 0)
        return WRITE_FILE;
    if(strncmp(commandMemorySpace[0], "delete_r", 8) == 0)
        return DELETE_RESOURCE_RECOURSIVLY;
    if(strncmp(commandMemorySpace[0], "delete", 6) == 0)
        return DELETE_RESOURCE;
    if(strncmp(commandMemorySpace[0], "find", 4) == 0)
        return FIND_RESOURCE;
    if(strncmp(commandMemorySpace[0], "exit", 4) == 0)
        return EXIT;
    return COMMAND_UNKNOWN;
}

int executeIOCommandInput(FileSystem_Node *root, FileSystem_HashMap **hashMap, int operation_code, String *parsedPath, String *parsedCommandInput, String extra)
{
    int operation_result = 0;
    FileSystem_HashMap *result = NULL;
    Data *file_data = NULL;

    switch(operation_code)
    {
        case CREATE_DIR :   operation_result = create_folder(root, parsedPath, parsedCommandInput, hashMap);
                            if(operation_result == TANGO_DOWN)
                                my_strcpy(extra, "ok", 2);
                            else
                                my_strcpy(extra, "no", 2);
                            return operation_result;

        case CREATE_FILE :  operation_result = create_file(root, parsedPath, parsedCommandInput, hashMap);
                            if(operation_result == TANGO_DOWN)
                                my_strcpy(extra, "ok", 2);
                            else
                                my_strcpy(extra, "no", 2);
                            return operation_result;

        case READ_FILE :    file_data = read_file(root, parsedCommandInput[1]);
                            if(file_data == (void *)ERRNO_DIRECTORY_DOES_NOT_EXIST || file_data == (void *)ERRNO_PATH_LENGTH_EXCEEDS_MAX_LENGTH)
                            {
                                my_strcpy(extra, "no", 2);
                                return ERRNO_DIRECTORY_DOES_NOT_EXIST;
                            }
                            my_strcpy(extra, "contenuto ", 10);
                            if(file_data != NULL)
                                strcat(extra, file_data->data);
                            return TANGO_DOWN;

        case WRITE_FILE :   operation_result = write_file(root, parsedCommandInput[1], parsedCommandInput[2]);
                            if(operation_result == TANGO_DOWN)
                                snprintf(extra, MAX_PATH_NAME_LENGTH, "ok %d", strlen(parsedCommandInput[2]));
                            else
                                my_strcpy(extra, "no", 2);
                            return operation_result;

        case DELETE_RESOURCE :  operation_result = delete_resource(root, parsedCommandInput[1], hashMap);
                                if(operation_result == TANGO_DOWN)
                                    my_strcpy(extra, "ok", 2);
                                else
                                    my_strcpy(extra, "no", 2);
                                return operation_result;

        case DELETE_RESOURCE_RECOURSIVLY :  operation_result = delete_resource_r(root, parsedCommandInput[1], hashMap);
                                            if(operation_result == TANGO_DOWN)
                                                my_strcpy(extra, "ok", 2);
                                            else
                                                my_strcpy(extra, "no", 2);
                                            return operation_result;

        case FIND_RESOURCE :    result = find_resource(hashMap, parsedCommandInput[1]);
                                if(result == NULL)
                                {
                                    my_strcpy(extra, "no", 2);
                                    return ERRNO_DIRECTORY_DOES_NOT_EXIST;
                                }
                                int index = 0;
                                FileSystem_HashMapValue *results = result->values;
                                String *paths = (String*) malloc(sizeof(String) * MAX_SUB_PATHS);
                                while(results != NULL)
                                {
                                    int length = strlen(results->path_name);
                                    paths[index] = (String) malloc(sizeof(char) * (length + 1));
                                    my_strcpy(paths[index], results->path_name, length);
                                    results = results->nextValue;
                                    index++;
                                }
                                sort(paths, index);
                                int i = 0;
                                for(; i < index; i++)
                                {
                                    strcat(extra, "ok ");
                                    strcat(extra, paths[i]);
                                    if(i < index - 1)
                                        strcat(extra, "\n");
                                }
                                free(paths);
                                return TANGO_DOWN;
        case EXIT : return EXIT;
        case ERRNO_CHAR_NOT_ALLOWED :   my_strcpy(extra, "no", 2);
                                        return ERRNO_CHAR_NOT_ALLOWED;
        case COMMAND_UNKNOWN :
        default :   my_strcpy(extra, "no", 2);
                    return ERRNO_COMMAND_UNKNOWN;

    }
}

int io_operationsLoop(FileSystem_Node *root, FileSystem_HashMap **hashMap)
{
    String commandInput = NULL;
    String *parsedIOCommands = NULL, *parsedPath = NULL;
    String commandOutput = (String) malloc(sizeof(char) * MAX_PATH_NAME_LENGTH);
    int operation_code = 0, operation_result = 1, counter = 0;
    commandOutput[0] = '\0';
    String commandMemorySpace[MAX_COMMAND_ARGS];
    commandMemorySpace[0] = (String) malloc(sizeof(char) * MAX_PATH_NAME_LENGTH);
    commandMemorySpace[1] = (String) malloc(sizeof(char) * MAX_PATH_NAME_LENGTH);
    commandMemorySpace[2] = (String) malloc(sizeof(char) * MAX_PATH_NAME_LENGTH);
    relativePath = (String) malloc(sizeof(char) * MAX_NAME_LENGTH);
    counter = 0;
    while(1)
    {
        commandInput = fetchIOCommandInput();
        operation_code = decodeIOCommandInput(commandInput, &parsedIOCommands, &parsedPath, commandMemorySpace);
        operation_result = executeIOCommandInput(root, hashMap, operation_code, parsedPath, parsedIOCommands, commandOutput);
        printf("%s", commandOutput);
        /*if(commandInput != NULL)
            free(commandInput); //just this because vfs_read allocate every time, needs to be freed*/
        //if(parsedPath != NULL)
        //    free(parsedPath);
        commandOutput[0] = '\0';
        if(operation_result ==  EXIT)
            return EXIT;
        printf("\n");
    }
}

