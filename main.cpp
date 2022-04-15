#include <iostream>
#include <sstream>
#include <pthread.h>
#include "fstream"
#include <getopt.h>
#include <map>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
//target_link_libraries(lab4 pthread)

using namespace std;

// Declaring new data type node
struct node {
    // Each Node will have data that is key and 2 pointers left and right indices
    int key;
    struct node *left, *right;
};

// Function to create new Node
struct node *newNode(int item) {
    auto *temp = (struct node *)malloc(sizeof(struct node));
    // point data to key
    temp->key = item;
    // point its left and right pointer to null pointer
    temp->left = temp->right = nullptr;
    // Return node
    return temp;
}

// This is function to print/traverse tree in inorder
void inorder(struct node *root) {
    if (root != nullptr) {
        // Traverse left
        inorder(root->left);

        // Traverse root
        cout << root->key << " -> ";

        // Traverse right
        inorder(root->right);
    }
}

struct thread_args {
    int value;
    node *root_ptr;
};

// Insert a node
struct node *insert(thread_args *temp_arg) {

    struct node *node = temp_arg->root_ptr;
    int key = reinterpret_cast<int>(temp_arg->value);

    // Return a new node if the tree is empty
    if (node == nullptr) return newNode(key);

    // Traverse to the right place and insert the node
    auto *temp = static_cast<thread_args *>(malloc(200));

    if (key < node->key){
        temp-> root_ptr = node->left;
        temp->value = key;
        node->left = insert(temp);
    }
    else{
        temp-> root_ptr = node->right;
        temp->value = key;
        node->right = insert(temp);
    }

    return node;
}

// Find the inorder successor
struct node *minValueNode(struct node *node) {
    struct node *current = node;

    // Find the leftmost leaf
    while (current && current->left != nullptr)
        current = current->left;

    return current;
}

// Deleting a node
struct node *deleteNode(thread_args *thread_arg) {
    struct node *root = thread_arg->root_ptr;
    int key = thread_arg->value;
    // Return if the tree is empty
    if (root == nullptr) return root;

    // Find the node to be deleted
    auto *temp_re_arg = static_cast<thread_args *>(malloc(200));

    if (key < root->key){
        temp_re_arg->value = key;
        temp_re_arg->root_ptr = root->left;
        root->left = deleteNode(temp_re_arg);
    }
    else if (key > root->key){
        temp_re_arg->value = key;
        temp_re_arg->root_ptr = root->right;
        root->right = deleteNode(temp_re_arg);
    }
    else {
        // If the node is with only one child or no child
        if (root->left == nullptr) {
            struct node *temp = root->right;
            free(root);
            return temp;
        } else if (root->right == nullptr) {
            struct node *temp = root->left;
            free(root);
            return temp;
        }

        // If the node has two children
        struct node *temp = minValueNode(root->right);

        // Place the inorder successor in position of the node to be deleted
        root->key = temp->key;
        temp_re_arg->value = temp->key;
        temp_re_arg->root_ptr = root->right;
        // Delete the inorder successor
        root->right = deleteNode(temp_re_arg);
    }
    return root;
}

struct node* search(struct node* root, int key){
    // Base Cases: root is null or key is present at root
    if (root == nullptr || root->key == key)
        return root;

    // Key is greater than root's key
    if (root->key < key)
        return search(root->right, key);

    // Key is smaller than root's key
    return search(root->left, key);
}




int main(int argc, char** argv) {

    // Create the variables to store your parameters
    std::map<std::string, std::string> input_parameters ;
    input_parameters["input"] = "default_in" ;   // Storage for input
    input_parameters["del_input"] = "default_out" ; // Storage for output

    // Create variables to hold your parameters
    const struct option longopts[] =
    {
        {"input", required_argument, 0, 'i'},
        {"del_input", required_argument, 0, 'd'},
        {0,0,0,0} // This tells getopt that this is the end
    };

    // Some parameters for getopt_long
    int c(0);

    // Get the options from the command line
    while (c != -1) {
        int option_index(-1) ;

        // Read the next command line option
        // Note here that the ':' after the 'i' and 'd' denotes that
        // it requires an argument
        c = getopt_long(argc, argv, "i:d:", longopts, &option_index) ;

        // If the option is valid, fill the corresponding value
        if ((c>0)&&(option_index>=0)) {
            std::cout << option_index << std::endl;
            input_parameters[longopts[option_index].name] = optarg ;
        }

        switch (c) {
            case 'i':
                // Fill input option
                input_parameters["input"] = optarg ;
            case 'd':
                // Fill output option
                input_parameters["del_input"] = optarg ;
            case '?':
                // getopt_long printed an error message
                break ;
        }
    }


    // Initiating root node
    struct node *root = nullptr;
    int delNums[3];

//    If you wanna set manual paths for  input and output files
///    string in_path = "/home/lucifer/CurrentTaskData/BST/inputs/in.txt";
///    string del_path = "/home/lucifer/CurrentTaskData/BST/inputs/temp.txt";

    pthread_t thr[8], del_thr[8];
    int ctr = 0;
    int temp_x = 0;

    cout << "\n\nReading input file and Inserting values to BST using 8 concurrent threads...\n";
    fstream input_file;
    input_file.open(input_parameters["input"],ios::in); //open a file to perform read operation using file object

    auto *in_args = static_cast<thread_args *>(malloc(200));
    auto *th_args = static_cast<thread_args *>(malloc(200));

    
    if (input_file.is_open()){
        string tp;
        while(getline(input_file, tp)){ //read data from file object and put it into string.
            stringstream temp(tp);
            temp >> temp_x;

            in_args-> value = reinterpret_cast<int>(temp_x);
            in_args-> root_ptr = root;

            if(ctr < 8){
                pthread_create(&thr[ctr], nullptr, reinterpret_cast<void *(*)(void *)>((node *) insert), (void*) th_args);
                root = insert(in_args);
                ctr = ctr + 1;
            }else{

                ctr = 0;
            }

        }
        input_file.close(); //close the file object.
    }

    cout << "Inorder traversal: ";
    inorder(root);
    
    //Nothing mentioned about insertion of delete file data so doing it by single thread
    cout << "\n\nReading delete number file and inserting numbers using single thread...\n";

    fstream new_file;
    new_file.open(input_parameters["del_input"],ios::in); //open a file to perform read operation using file object
    if (new_file.is_open()){ //checking whether the file is open
        string tp;
        int counter = 0;
        while(getline(new_file, tp)){ //read data from file object and put it into string.
            if (counter < 3){
                stringstream temp(tp);
                int temp_x = 0;
                temp >> temp_x;
                delNums[counter] = temp_x;
            } else{
                break;
            }
            counter++;
        }
        new_file.close(); //close the file object.
    }

    for (int delNum : delNums) {
        auto *temp_del_args = static_cast<thread_args *>(malloc(200));
        temp_del_args->value = delNum;
        temp_del_args->root_ptr = root;

        root = insert(temp_del_args);
    }

    cout << "Inorder traversal: ";
    inorder(root);

    cout << "\n\nSearching...\n";
    for (int i: delNums) {
        if (search(root, i)){
            cout << i << ":Found\n";
        }else{
            cout << i << ":Not Found\n";
        }
    }

    int del_ctr = 0;

    cout << "\n\nDeleting using 8 concurrent threads...";
    for(int i: delNums){

        auto *del_args = static_cast<thread_args *>(malloc(500));

        del_args-> value = reinterpret_cast<int>(i);
        del_args-> root_ptr = root;

        if(del_ctr < 8){
            pthread_create(&del_thr[del_ctr], nullptr, reinterpret_cast<void *(*)(void *)>((node *) deleteNode), (void*) del_args);
            root = deleteNode(del_args);
            del_ctr++;
        }else{
            for (pthread_t tem: del_thr){
                pthread_join(tem, nullptr);
            }
            del_ctr = 0;
        }
    }
    cout << "\nInorder traversal: ";
    inorder(root);

    cout << "\n\nSearching...\n";
    for (int i: delNums) {
        if (search(root, i)){
            cout << i << ":Found\n";
        }else{
            cout << i << ":Not Found\n";
        }
    }
}