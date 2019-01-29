#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*構造体を定義*/
typedef struct _node{
	char postalCode[10];
	char address[256];
	struct _node* parent;
	struct _node* left;
	struct _node* right;
}Node;

/*検索結果リストを保持するためのリスト構造*/
typedef struct _list{
	Node* pNode;
	struct _list* next;
	/*住所をリスト表示する際のリスト番号*/
	int listNum;
}List;

/*構造体Nodeの先頭アドレスと末端アドレス*/
Node* rootNode = NULL;
Node* end = NULL;

/*構造体Listの先頭アドレス*/
List* rootList = NULL;
List* list = NULL;
int* g_p = NULL;
int matchJudge = 0;
int addFlag = 0;

/*プロトタイプ宣言*/
Node *makeNode(Node* node,char* filePostalCode,char* fileAddress,Node* parent);
List *searchPostalCode(char* search,Node* node,int searchSize);
Node *getEndOfTree(Node *node);
List *findList(int inputListNum,List* list);
void delete(Node* node);
void change(Node* node,char* inputAddress);
void releaseMemory(Node* node);
void replaceNode(Node* node,int nodeCase);
void myMalloc(int size);

int main(void){
	/*変数定義*/	
	/*ファイルポインタ*/
	FILE* fp = NULL;
	/*ファイルパス*/
	char* fname = "KEN_ALL_JPN.csv";
	int ret = 0;
	Node *node = NULL;

	/*ファイルから読み込んだデータを格納する変数*/
	char filePostalCode[9] = {0};
	char fileAddress[256] = {0};

	/*ユーザーから受け取る入力値*/
	/*モード*/
	char mode;
	/*郵便番号*/
	char inputPostalCode[9] = {0};
	/*住所*/
	char inputAddress[256] = {0};
	/*検索する郵便番号*/
	char search[9] = {0};
	/*searchの要素数*/
	int searchSize = 0;
	/*住所検索結果のリストメモリ解放用のバッファ*/
	List* bufMem = NULL;
	/*住所を新規挿入する場合に、同一郵便番号は弾くための、追加モード判別フラグ*/

	/*カウント用*/
	int count = 0;
	/*編集したいリスト番号*/
	int inputListNum = 0;

	/*ファイルを開く*/
	fp = fopen(fname,"r");

	if (fp == NULL){
		printf("%sファイルは開けません！！\n",fname);	
		return -1;
	}
	
	/*ファイルから1行ずつデータを読み込み、各データを配列に格納していく*/
	while ((ret = fscanf(fp,"%[^,],%s ",filePostalCode,fileAddress)) != EOF){
		node = makeNode(rootNode,filePostalCode,fileAddress,NULL);	
		if(rootNode == NULL){
			rootNode = node;
		}
	}

	/*ファイルを閉じる*/
	fclose(fp);
	fp = NULL;	
	/*ここからユーザー入力を受付*/
	/*検索モード*/
	while(1){
		printf("current postalcode of rootNode is %s\n",rootNode);
		printf("please choose mode...\na...add \ns...search\nor quit? please input q\n");
		scanf(" %c",&mode);
		fflush(stdin);
		if(mode == 'q'){
			/*終了モードが渡ってきたら、動的確保した分のメモリをせめて解放して終了*/	
			if(rootNode != NULL){
				node = rootNode;
				releaseMemory(node);
				printf("allocated memory is released!\n");
			}
			printf("Bye!\n");
			return 0;
		}else if(mode == 'a'){
			/*追加モード*/	
			addFlag = 1;
			printf("please input new postal code\n");
			scanf("%s",inputPostalCode);
			//fgets(inputPostalCode,sizeof(inputPostalCode),stdin);
			printf("please input new address\n");
			scanf("%s",inputAddress);
			//fgets(inputAddress,sizeof(inputAddress),stdin);
			makeNode(rootNode,inputPostalCode,inputAddress,NULL);
		}else if(mode == 's'){
			/*検索モード*/
			printf("please input postalCode which you wanna search\n");
			fgets(search,sizeof(search),stdin);
			/*searchの要素数を取得する。*/
			searchSize = strlen(search) - 1;
			/*検索結果のノードのアドレスを格納*/
			rootList = searchPostalCode(search,rootNode,searchSize);
			if(rootList != NULL){
				list = rootList;
				count = 0;
				while(count < 100 && list != NULL){
					count++;
					printf("No.%d\n",count);
					printf("%s\n",list->pNode->postalCode);
					printf("%s\n",list->pNode->address);
					printf("parent == %s\n",list->pNode->parent);
					printf("right == %s\n",list->pNode->right);
					printf("left == %s\n",list->pNode->left);
					printf("-----------------------------------------\n");			
					list->listNum = count;
					list = list->next;
				}

				/*検索モードの状態から、削除モードか変更モードに任意で移動させる*/
				printf("Do you want to delete or change data?\nd...delete\nc...change\nq...quit\n");
				scanf(" %c",&mode);

				if(mode == 'd'){
					/*削除モード*/	
					printf("please choose list number whitch you want to delete.\n");
					printf("For Example, NO.1>>>1\n");
					scanf("%d",&inputListNum);
					list = findList(inputListNum,rootList);
					if(list == NULL){
						printf("Sorry. this list number is none..\n");	
					}else{
						delete(list->pNode);
						list->pNode = NULL;
					}
				}else if(mode == 'c'){
					/*変更モード*/
					printf("please choose list number whitch you want to change.\n");
					printf("For Example, NO.1>>>1\n");
					scanf("%d",&inputListNum);
					list = findList(inputListNum,rootList);
					if(list == NULL){
						printf("Sorry. this list number is none..\n");	
					}else{
						change(list->pNode,inputAddress);
						list->pNode = NULL;
					}
				}else if(mode == 'q'){
					/*この処理をスキップ*/	
				}

				/*動的にメモリを確保したリストはここで解放*/
				if(rootList != NULL){
					list = rootList;
					while(list != NULL){
						bufMem = list->next;
						free(list);	
						list = bufMem;
					}
					rootList = NULL;
				}
			}else{
				printf("Not Found...\n");	
			}
		}
		fflush(stdin);
	}
}

/*ファイルから読み込んだデータでノードを作成する関数*/
Node *makeNode(Node *node,char* filePostalCode,char* fileAddress,Node* parent){
	/*ポインタがNULLであるか確認*/
	if(node == NULL){
		/*メモリを動的に確保*/
		/*動的確保したメモリがNULLでないことを確認*/
		myMalloc(sizeof(Node));

		node = (Node *)g_p;

		/*各データを格納*/
		strcpy(node->postalCode,filePostalCode);
		strcpy(node->address,fileAddress);
		if(addFlag == 1){
			printf("%s\n",node->postalCode);
			printf("%s\n",node->address);
		}

		/* 左右のポインタをNULLにする*/
		node->left = NULL;
		node->right = NULL;
		node->parent = parent;
	}else{
		matchJudge = strcmp(filePostalCode,node->postalCode);
		if(matchJudge == 0 && addFlag == 1){
			printf("You can't regestar same values.\n");	
			return node;
		}
		/*読み込んだデータとノードのデータを比較し、左右のブランチに振り分ける*/	
		if(matchJudge <= 0){
			/*親ノードに現在のノードのアドレス渡し、左ノードを次のノードに指定*/
			parent = node;
			node->left = makeNode(node->left,filePostalCode,fileAddress,parent);
		}else if(matchJudge > 0){
			/*親ノードに現在のノードのアドレス渡し、右ノードを次のノードに指定*/
			parent = node;
			node->right = makeNode(node->right,filePostalCode,fileAddress,parent);
		}
	}
	/*ファイルから読み込んだ一行分のアドレスと住所を次の一行のデータの領域を確保するために初期化をする*/
	memset(filePostalCode,0,sizeof(filePostalCode));
	memset(fileAddress,0,sizeof(fileAddress));

	return node;
}

/*郵便番号検索の関数*/
List *searchPostalCode(char* search,Node* node,int searchSize){
	if(node == NULL){
		return NULL;	
	}else{
		/*受けとった数値が見つかるまで、rootNodeから順にノードを検索*/	
		matchJudge = strncmp(search,node->postalCode,searchSize);
		if(matchJudge == 0){
			/*ユーザーから受け取った郵便番号と今見ているノードの郵便番号が一致していれば、ノードのアドレスを返す*/
			/*メモリを動的に確保*/
			if(rootList == NULL){
				myMalloc(sizeof(List));
				list = (List *)g_p;
				rootList = list;
			}else{
				myMalloc(sizeof(List));
				list->next = (List *)g_p;
				list = list->next;
			}

			/*構造体listのrootListに、検索結果として該当する構造体Nodeのアドレスを格納する*/
			list->pNode = node;
			/*次の構造体へのアドレスをNULLに*/
			list->next = NULL;
		}				

		if(node->left != NULL){
			searchPostalCode(search,node->left,searchSize);
		}
		if(node->right != NULL){
			searchPostalCode(search,node->right,searchSize);
		}
		if(node == rootNode){
			return rootList;
		}
	}
}

/*データ削除の関数*/
void delete(Node* node){
	/*削除対象のノードが空の時はNULLを返す*/
	if(node == NULL){
		printf("this postalcode is none\n");	
	}else if(node->left == NULL && node->right == NULL){
		/*左右ノードがNULLの場合は親ノードからみて該当する左右どちらかのポインタをNULLにし、ノードを開放*/	
		replaceNode(node,1);
	}else if(node->left == NULL){
		/*左ノードのみがNULLの場合は親ノードからみて該当する左右どちらかのポインタをノードの左ポインタにし、ノードを開放*/	
		replaceNode(node,2);
	}else if(node->right == NULL){
		/*右ノードのみがNULLの場合は親ノードからみて該当する左右どちらかのポインタをノードの右ポインタにし、ノードを開放*/	
		replaceNode(node,3);
	}else{
		/*左右どちらのノードも値がある場合、右ノードから左ノードをNULLがくるまで探索し、末端を見つける*/
		end = getEndOfTree(node->right);	
		replaceNode(end,4);
		/*現在のノードに末端のノードのデータをコピーする*/
		strcpy(node->postalCode,end->postalCode);
		strcpy(node->address,end->address);

		/*末端のメモリを開放*/
		free(end);
		end = NULL;
	}
	printf("this data is deleted\n");
}

/*データ変更の関数*/
void change(Node* node,char* inputAddress){
	/*新しい住所を入力！*/
	printf("please new address\n");
	scanf("%s",inputAddress);

	/*ノードに既にあるデータを0で初期化*/
	memset(node->address,0,sizeof(node->address));

	/*新しいデータをノードのデータにコピー*/
	strcpy(node->address,inputAddress);

	/*受け取った入力値を0で初期化*/
	memset(inputAddress,0,sizeof(inputAddress));
}

/*ツリーの末端で最小値のもの（rootNodeの右ノードから探す）を探す*/
Node *getEndOfTree(Node* node){
	/*ルートの右ノードから最小値を探していく*/
	while(node->left != NULL){
		node = node->left;
	}
	return node;
}
		
/*メモリ解放の関数*/
void releaseMemory(Node* node){
	/*左右のブランチを見て、ノードが存在していれば再起処理で呼び出し*/
	if(node->left != NULL){
		releaseMemory(node->left);
	}else if(node->right != NULL){
		releaseMemory(node->right);
	}
	/*ノードが存在していなければ、releaseに現在のノードのアドレスを保持し、nodeが指すメモリを開放*/
	free(node);
	node = NULL;
}

/*削除処理の中のメモリ解放とノードの付け替え処理*/
void replaceNode(Node* node,int nodeCase){
	if(node->parent != NULL){
		matchJudge = strcmp(node->parent->postalCode,node->postalCode);
		if(matchJudge > 0){
			if(nodeCase == 2){
				node->parent->right = node->right;	
				if(node->right != NULL){
					node->right->parent = node->parent;
				}
			}else if(nodeCase == 3 || nodeCase == 1 || nodeCase == 4){
				node->parent->left = node->left;	
				if(node->left != NULL){
					node->left->parent = node->parent;
				}
			}
		}else if(matchJudge < 0){
			if(nodeCase == 2 || nodeCase == 1 || nodeCase == 4){
				node->parent->right = node->right;	
				if(node->right != NULL){
					node->right->parent = node->parent;
				}
			}else if(nodeCase == 3){
				node->parent->left = node->left;	
				if(node->left != NULL){
					node->left->parent = node->parent;
				}
			}
		}
	}else{
		if(nodeCase == 1){
			rootNode = NULL;	
		}else if(nodeCase == 2){
			rootNode = node->right;	
			node->right->parent = NULL;
		}else if(nodeCase == 3 || nodeCase == 4){
			rootNode = node->left;	
			node->left->parent = NULL;
		}
	}
	free(node);
	node = NULL;
}

/*動的確保したメモリのぬるぽチェック*/
void myMalloc(int size){
	g_p = malloc(size);
	if(g_p == NULL){
		printf("null Po\n");
		exit(1);
	}else{
		/*確保したメモリの値を0で初期化*/	
		memset(g_p,0,size);
	}
}

/*削除と編集機能選択時、入力されたリスト番号から該当リストを探し、そのリストのアドレスを返す*/
List *findList(int inputListNum,List* list){	
	while(list != NULL){
		if(inputListNum == list->listNum){
			break;
		}	
		list = list->next;
	}
	return list;
}
