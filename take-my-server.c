/*SERVIDOR*/
/********/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/*API*/
#include <curl/curl.h>
#include <json-c/json.h>

typedef struct {
    unsigned char *buffer;
    size_t len;
    size_t buflen;
} get_request;

#define MAX_CONN 100     //Nº máximo de conexiones en espera
#define MAX_TAM_MENSAJE 512 //Numero de caracteres maximo del mensaje
#define CHUNK_SIZE 2048

// para usar la api
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb; 
    get_request *req = (get_request *) userdata;

    printf("receive chunk of %zu bytes\n", realsize);

    while (req->buflen < req->len + realsize + 1)
    {
        req->buffer = realloc(req->buffer, req->buflen + CHUNK_SIZE);
        req->buflen += CHUNK_SIZE;
    }
    memcpy(&req->buffer[req->len], ptr, realsize);
    req->len += realsize;
    req->buffer[req->len] = 0;

    return realsize;
}
// fin para usar la api

int puerto_id, coneccion_id;

/**********************************************************/
/* función catch que captura una interrupción             */
/**********************************************************/
void catch(int sig)
{
	printf("***Señal: %d atrapada!\n", sig);
  printf("***Cerrando servicio ...\n");
  close(coneccion_id);
  close(puerto_id);
  printf("***Servicio cerrado.\n");
  exit(EXIT_SUCCESS);
}

/**********************************************************/
/* función MAIN                                           */
/* Orden Parametros: Puerto                               */
/**********************************************************/

int main(int argc, char *argv[]) {

  socklen_t destino_tam;
  struct sockaddr_in origen_dir, destino_dir;
  char mensaje_entrada[MAX_TAM_MENSAJE], mensaje_salida[MAX_TAM_MENSAJE];

  if (argc != 2) {
    printf("\n\nEl número de parámetros es incorrecto\n");
    printf("Use: %s <puerto>\n\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  // Creamos el socket del servidor
	puerto_id = socket(AF_INET,SOCK_STREAM,0);
	if (puerto_id == -1) {
    printf("ERROR: El socket del servidor no se ha creado correctamente!\n");
    exit(EXIT_FAILURE);
  }

  //Se prepara la dirección de la máquina servidora
  origen_dir.sin_family = AF_INET;
  origen_dir.sin_port = htons(atoi(argv[1]));
  origen_dir.sin_addr.s_addr = htonl(INADDR_ANY);

  //Asigna una direccion local al socket
	if( bind(puerto_id, (struct sockaddr*)&origen_dir, sizeof(origen_dir)) == -1) {
    printf("ERROR al unir el socket a la dirección de la máquina servidora\n");
    close(puerto_id);
    exit(EXIT_FAILURE);
  }

  //Espera al establecimiento de alguna conexión de multiples usuarios
  if( listen(puerto_id, MAX_CONN) == -1) {
    printf("ERROR al establecer la escucha de N conecciones en el servidor\n");
    close(puerto_id);
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, &catch);
  while(1){
    printf("\n***Servidor ACTIVO escuchando en el puerto: %s ...\n",argv[1]);
    //Establece una conexión
    destino_tam=sizeof(destino_dir);
    coneccion_id = accept(puerto_id, (struct sockaddr*)&destino_dir, &destino_tam);
    if(coneccion_id == -1) {
      printf("ERROR al establecer la conección del servidor con el cliente\n");
      close(puerto_id);
      exit(EXIT_FAILURE);
    }
    printf("***Servidor se conecto con el cliente: %d.\n",destino_dir.sin_addr.s_addr);
    do {
      //Recibe el mensaje del cliente
      if (recv(coneccion_id, mensaje_entrada, sizeof(mensaje_entrada), 0) == -1) {
        perror("Error en recv");
        close(coneccion_id);
        close(puerto_id);
        exit(EXIT_SUCCESS);
      } else
			   printf("<<Client envía >>: %s\n", mensaje_entrada);
         // Inicio api
          CURL *curl;
          CURLcode res;
          curl = curl_easy_init();
          struct json_object *parsed_json;
          struct json_object *result;
          char valor[100] = "", convertir[100] = "", convertido[100] = "", respuesta[1000] = "", ans[1000] = "";

          //separar string
          int i=0, j=0, ctr=0;
          for(i=0;i<(strlen(mensaje_entrada))-1;i++)
          {
              // if space or NULL found, assign NULL into newString[ctr]
              if(mensaje_entrada[i]==' '||mensaje_entrada[i]=='\0')
              {
                  
                  ctr++;  //for next word
                  j=0;    //for next word, init index to 0
              }
              else
              {
                  if(ctr == 0){
                      valor[j] = mensaje_entrada[i];
                  }else if(ctr == 1){
                      convertir[j] = mensaje_entrada[i];
                  }else if(ctr == 2){
                      convertido[j] = mensaje_entrada[i];
                  }
                  j++;
              }
          }
          strcat(respuesta, "https://api.apilayer.com/exchangerates_data/convert?to=");
          strcat(respuesta, convertido);
          strcat(respuesta, "&from=");
          strcat(respuesta, convertir);
          strcat(respuesta, "&amount=");
          strcat(respuesta, valor);
          // strncpy(ans, strchr(respuesta, 'h'), strlen(respuesta)-1);
          //fin separar string
          // printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaa\n");

          get_request req = {.buffer = NULL, .len = 0, .buflen = 0};

          if (curl) {
              curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
              // printf("%s", ans);
              // curl_easy_setopt(curl, CURLOPT_URL, "https://api.apilayer.com/exchangerates_data/convert?to=CLP&from=ARS&amount=879");
              printf("Respuesta: %s\n", respuesta);
              curl_easy_setopt(curl, CURLOPT_URL, respuesta);
              printf("Si pasa\n");
              curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
              curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
              struct curl_slist *headers = NULL;
              headers = curl_slist_append(headers, "apikey: 9COfh6HcQz1tPC6M67W17Eqz1wv6r0vq");
              curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

              req.buffer = malloc(CHUNK_SIZE);
              req.buflen = CHUNK_SIZE;

              curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
              curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&req);

              res = curl_easy_perform(curl);

              parsed_json = json_tokener_parse(req.buffer);
              json_object_object_get_ex(parsed_json, "result", &result);
              printf("result: %s\n", json_object_get_string(result));
              free(req.buffer);      
              
                
          }

          curl_easy_cleanup(curl);
         // Fin api

      //Envia el mensaje al cliente
		  sprintf(mensaje_salida, "El mensaje recibido fue --- %s ---.",json_object_get_string(result));
		  if (send(coneccion_id, strcat(mensaje_salida,"\0"), strlen(mensaje_salida)+1, 0) == -1) {
        perror("Error en send");
        close(coneccion_id);
        close(puerto_id);
        exit(EXIT_SUCCESS);
      } else
        printf("<<Server replica>>: %s\n", json_object_get_string(result));
    }while(strcmp(mensaje_entrada,"terminar();") != 0);

    //Cierra la conexión con el cliente actual
    printf("***Cerrando conección con cliente ...\n");
    close(coneccion_id);
    printf("***Conección cerrada.\n");
  }
  //Cierra el servidor
  printf("***Cerrando servicio ...\n");
  close(puerto_id);
  printf("***Servicio cerrado.\n");
  exit(EXIT_SUCCESS);
}