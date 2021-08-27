#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <libxml/xpathInternals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#define SERVER_PATH "/tmp/server"
#define BUFFER_LEN 	1024
#define FILE_NAME 	"example.xml"

/* 	
	The implementation of the program below, specifically parts that make use of the libxml library and its
	functions closely follow from the xpath examples available at xmlsoft.org
	
	The UNIX socket programming method utilized here is fairly common, although the implementation below was 
	inspired by a combination of sources including stackoverflow, IBM, and opensource.com
*/ 

int init_server_socket();
int cli_set(const char *filename, char *xpathExpr, const xmlChar *value);
char * cli_get(const char *filename, char *xpathExpr);
void update_xpath_nodes(xmlNodeSetPtr nodes, const xmlChar* value);
char * get_content(xmlNodeSetPtr nodes);

int main(int argc, char **argv) {

	int sd0 = -1, sd1 = -1;
	int rcv, bind_flag = -1;
	int listen_flag, length;
	char cmd[128]; // command buffer
	char buf[BUFFER_LEN], msg[BUFFER_LEN];
	char *input, *get_result;
	struct sockaddr_un address;

	do {
		sd0 = socket(AF_UNIX, SOCK_STREAM, 0);
		if (sd0 < 0) {
			perror("Socket initiation failed.");
			break;
		}
		unlink(SERVER_PATH); // in case anything is binded to the address
		memset(&address, 0, sizeof(address));
		address.sun_family = AF_UNIX;
		strcpy(address.sun_path, SERVER_PATH);
		
		bind_flag = bind(sd0, (struct sockaddr *)&address, SUN_LEN(&address));
		if (bind_flag < 0) {
			perror("Binding failed");
			break;
		}
		
		printf("Up & running.\n");
		
		listen_flag = listen(sd0, 1);
		if (listen_flag < 0) {
			perror("Listen function failed");
			break;
		}
		
		sd1 = accept(sd0, NULL, NULL);
		if (sd1 < 0) {
			perror("Accept function failed");
			break;
		}
	} while (0);
	
	xmlInitParser();
	
	while (1) {
		//memset(buf, 0, sizeof(char)*BUFFER_LEN);
		
		rcv = recv(sd1, buf, sizeof(buf), 0);
		printf("Received %d bytes of data.\n", rcv);
		
		if (rcv == 0 || rcv < sizeof(buf)) {
			printf("The client closed the connection.\n");
			break;
		}

		printf("Received message: %s\n", buf);
		
		char *path, *val;
		input = strtok(buf, " ");
		path = strtok(NULL, " ");
		
		if (strcmp(input, "cli_set") == 0) {
			val = strtok(NULL, " ");
			printf("%s\npath: %s\nnew value: %s\n", input, path, val);
			if (cli_set(FILE_NAME, path, BAD_CAST val)) {
				printf("Failed\n");
				return(-1);
			}
			strcpy(msg, "Set operation successful.");
			rcv = send(sd1, msg, sizeof(msg), 0);
		}
		
		if (strcmp(input, "cli_get") == 0) {
			printf("MAIN-APP RECEIVED CLI_GET AND IS MAKING THE CALL.\n");
			printf("%s\npath: %s\n", input, path);
			get_result = cli_get(FILE_NAME, path);
			if (get_result == NULL) {
				printf("Failed\n");
				return(-1);
			}
			strcpy(msg, get_result);
			rcv = send(sd1, msg, sizeof(msg), 0);
		}
		
		if (strcmp(input, "cli_run") == 0) {
			int flag;
			char *args[2] = {path, NULL};
			flag = execvp(path, args);
			if(flag == -1) 
				printf("Could not run\n");
			strcpy(msg, "Run operation successful.");
			rcv = send(sd1, msg, sizeof(msg), 0);
		}
		
		if (strcmp(input, "cli_exec") == 0) {
			int flag;
			char *args[2] = {path, NULL};
			flag = execvp(path, args);
			if(flag == -1) 
				printf("Could not run\n");
			strcpy(msg, "Exec operation successful.");
			rcv = send(sd1, msg, sizeof(msg), 0);
		}
	}
	
			
	if (sd0 != -1)
		close(sd0);
	if (sd1 != -1)
		close(sd1);
		
	unlink(SERVER_PATH);
	printf("Socket closed.\n");

	xmlCleanupParser();
	xmlMemoryDump();
	return 0;
}

char * cli_get(const char *filename, char *xpathExpr) {
    
	xmlDocPtr doc;
	xmlNode *cur_node;
	xmlXPathContextPtr context;
	xmlXPathObjectPtr xpathObj;
	char *node = {0};
	char new_path[128] = "";
	char *content;
    
    doc = xmlParseFile(filename);
    if (doc == NULL) {
		printf("Unable to parse file: %s\n", filename);
		return NULL;
    }

    context = xmlXPathNewContext(doc);
    if(context == NULL) {
        printf("Unable to create new context\n");
        xmlFreeDoc(doc);
        return NULL;
    }
	
	node = strtok(xpathExpr, ".");
	
	while (node != NULL) {
		printf("Token: %s\n", node);
		strncat(new_path, "//", 32);
		strncat(new_path, node, 32);
		node = strtok(NULL, ".");
	}
	
	//printf("PATH: %s\n", new_path);
	
	xpathObj = xmlXPathEvalExpression(BAD_CAST new_path, context);
	if(xpathObj == NULL) {
		printf("Unable to evaluate xpath expression: %s\n", xpathExpr);
		xmlXPathFreeContext(context);
		xmlFreeDoc(doc);
		return NULL;
	}

    //cur_node = xpathObj->nodesetval->nodeTab[0];
    //content = (char *) xmlNodeGetContent(cur_node);
    printf("PRINTING:\n%s\n", content);
    xmlNodeSetPtr nodes;
    nodes = xpathObj->nodesetval;
    //printf("NUMBER OF NODES: %d\n\n", size);
	content = (char *) xmlNodeGetContent(nodes->nodeTab[0]);
	
	printf("CONTENT: %s\n\n", content);

    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(context); 
    //xmlDocDump(stdout, doc);
    xmlFreeDoc(doc); 
    
    return content;
}

int cli_set(const char *filename, char *xpathExpr, const xmlChar *value) {
    
	xmlDocPtr doc;
	xmlXPathContextPtr context;
	xmlXPathObjectPtr xpathObj;
	char *node = {0};
	char new_path[128] = "";
    
    doc = xmlParseFile(filename);
    if (doc == NULL) {
		printf("Unable to parse file: %s\n", filename);
		return(-1);
    }

    context = xmlXPathNewContext(doc);
    if(context == NULL) {
        printf("Unable to create new context\n");
        xmlFreeDoc(doc);
        return(-1);
    }
	
	node = strtok(xpathExpr, ".");
	
	while (node != NULL) {
		printf("Token: %s\n", node);
		strncat(new_path, "//", 32);
		strncat(new_path, node, 32);
		node = strtok(NULL, ".");
	}
	
	printf("PATH: %s\n", new_path);
	
	xpathObj = xmlXPathEvalExpression(BAD_CAST new_path, context);
	if(xpathObj == NULL) {
		printf("Unable to evaluate xpath expression: %s\n", xpathExpr);
		xmlXPathFreeContext(context);
		xmlFreeDoc(doc);
		return(-1);
	}
	
    update_xpath_nodes(xpathObj->nodesetval, value);

    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(context); 
	
	//xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
	//xmlSaveFormatFileEnc(docout, doc, "UTF-8", 1);
    xmlDocDump(stdout, doc);
    xmlFreeDoc(doc); 
    
    return(0);
}

void update_xpath_nodes(xmlNodeSetPtr nodes, const xmlChar* value) {
    int size;
    int i;
    
    size = (nodes) ? nodes->nodeNr : 0;
    
    for(i = size - 1; i >= 0; i--) {
		xmlNodeSetContent(nodes->nodeTab[i], value);
	
		if (nodes->nodeTab[i]->type != XML_NAMESPACE_DECL)
			nodes->nodeTab[i] = NULL;
		}
}


