#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "manual.h"

void Init_person(Person *p, const char *name, const char *last_name, 
                const char *patronymic, const char *workplace, const char *job) {
    p->name = strdup(name);
    p->last_name = strdup(last_name);
    p->patronymic = patronymic ? strdup(patronymic) : NULL;
    p->job = job ? strdup(job) : NULL;
    p->workplace = workplace ? strdup(workplace) : NULL;
}

void Free_person(Person *p) {
    if (!p) return;
    free(p->name);
    free(p->last_name);
    free(p->patronymic);
    free(p->job);
    free(p->workplace);
}

void Init_contact(Contact *contact) {
    if (!contact) return;
    contact->emails = malloc(sizeof(Email) * SIZE);
    contact->phone_numbers = malloc(sizeof(Phone_number) * SIZE);
    contact->socials = malloc(sizeof(Social) * SIZE);
    contact->email_counter = 0;
    contact->phone_numbers_counter = 0;
    contact->socials_counter = 0;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int height(Contact *node) {
    return node ? node->height : 0;
}

void updateHeight(Contact *node) {
    if(node) node->height = 1 + max(height(node->left), height(node->right));
}

Contact* rotateRight(Contact *y) {
    Contact *x = y->left;
    Contact *T2 = x->right;

    x->right = y;
    y->left = T2;

    updateHeight(y);
    updateHeight(x);

    return x;
}

Contact* rotateLeft(Contact *x) {
    Contact *y = x->right;
    Contact *T2 = y->left;

    y->left = x;
    x->right = T2;

    updateHeight(x);
    updateHeight(y);

    return y;
}

int getBalance(Contact *node) {
    return node ? height(node->left) - height(node->right) : 0;
}

int compareContacts(Contact *a, Contact *b) {
    int cmp = strcmp(a->person.last_name, b->person.last_name);
    if(cmp == 0) {
        cmp = strcmp(a->person.name, b->person.name);
    }
    return cmp;
}

Contact* insertNode(Contact *node, Contact *newContact, Manual *manual) {
    if(!node) return newContact;

    int cmp = compareContacts(newContact, node);
    if(cmp < 0)
        node->left = insertNode(node->left, newContact, manual);
    else
        node->right = insertNode(node->right, newContact, manual);

    updateHeight(node);

    int balance = getBalance(node);

    if(balance > 1 && compareContacts(newContact, node->left) < 0)
        return rotateRight(node);

    if(balance < -1 && compareContacts(newContact, node->right) > 0)
        return rotateLeft(node);

    if(balance > 1 && compareContacts(newContact, node->left) > 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }

    if(balance < -1 && compareContacts(newContact, node->right) < 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

void Init_Manual(Manual *manual) {
    manual->root = NULL;
    manual->id_counter = 0;
    manual->operations_counter = 0;
}

void Free_contact(Contact *contact) {
    if(!contact) return;
    
    free(contact->person.name);
    free(contact->person.last_name);
    free(contact->person.patronymic);
    free(contact->person.job);
    free(contact->person.workplace);
    
    for(int i = 0; i < contact->email_counter; i++)
        free(contact->emails[i].email);
    free(contact->emails);
    
    for(int i = 0; i < contact->phone_numbers_counter; i++)
        free(contact->phone_numbers[i].phone_number);
    free(contact->phone_numbers);
    
    for(int i = 0; i < contact->socials_counter; i++)
        free(contact->socials[i].social);
    free(contact->socials);
    
    free(contact);
}

Contact* findContactById(Contact *root, int id) {
    if(!root) return NULL;
    
    if(root->ID == id) return root;
    
    Contact *left = findContactById(root->left, id);
    if(left) return left;
    
    return findContactById(root->right, id);
}

void Add_contact_manual(Manual *manual, const char *name, const char *last_name, 
                const char *patronymic, const char *workplace, const char *job) {
    Contact *newContact = malloc(sizeof(Contact));
    newContact->ID = manual->id_counter++;
    Init_person(&newContact->person, name, last_name, patronymic, workplace, job);
    Init_contact(newContact);
    newContact->left = newContact->right = NULL;
    newContact->height = 1;

    manual->root = insertNode(manual->root, newContact, manual);
    manual->operations_counter++;
    
    if(manual->operations_counter % BALANCE_FREQUENCY == 0) {
        BalanceTree(manual);
    }

}

void storeContactsInArray(Contact *node, Contact **contacts, int *index) {
    if(!node) return;
    storeContactsInArray(node->left, contacts, index);
    contacts[(*index)++] = node;
    storeContactsInArray(node->right, contacts, index);
}

Contact* buildBalancedTree(Contact **contacts, int start, int end) {
    if(start > end) return NULL;
    
    int mid = (start + end) / 2;
    Contact *root = contacts[mid];
    
    root->left = buildBalancedTree(contacts, start, mid-1);
    root->right = buildBalancedTree(contacts, mid+1, end);
    
    return root;
}

void countNodes(Contact *node, int *count) {
    if(node) {
        countNodes(node->left, count);
        (*count)++;
        countNodes(node->right, count);
    }
}

void BalanceTree(Manual *manual) {
    int count = 0;
    countNodes(manual->root, &count);
    
    Contact **contacts = malloc(count * sizeof(Contact*));
    int index = 0;
    storeContactsInArray(manual->root, contacts, &index);
    
    manual->root = buildBalancedTree(contacts, 0, count-1);
    free(contacts);
}

Contact* minValueNode(Contact *node) {
    Contact *current = node;
    while(current && current->left)
        current = current->left;
    return current;
}

void CopyContactData(Contact *dest, Contact *src) {
    dest->ID = src->ID;
    Init_person(&dest->person, src->person.name, src->person.last_name, 
                src->person.patronymic, src->person.workplace, src->person.job);

    dest->emails = malloc(SIZE * sizeof(Email));
    for(int i = 0; i < src->email_counter; i++) {
        dest->emails[i].email = strdup(src->emails[i].email);
    }
    dest->email_counter = src->email_counter;

    dest->phone_numbers = malloc(SIZE * sizeof(Phone_number));
    for(int i = 0; i < src->phone_numbers_counter; i++) {
        dest->phone_numbers[i].phone_number = strdup(src->phone_numbers[i].phone_number);
    }
    dest->phone_numbers_counter = src->phone_numbers_counter;

    dest->socials = malloc(SIZE * sizeof(Social));
    for(int i = 0; i < src->socials_counter; i++) {
        dest->socials[i].social = strdup(src->socials[i].social);
    }
    dest->socials_counter = src->socials_counter;
}

Contact* deleteNode(Contact *root, int id, Manual *manual) {
    if (!root) return root;

    if (id < root->ID) {
        root->left = deleteNode(root->left, id, manual);
    } else if (id > root->ID) {
        root->right = deleteNode(root->right, id, manual);
    } else {
        if (!root->left || !root->right) {
            Contact *temp = root->left ? root->left : root->right;
            
            if (!temp) {
                Free_contact(root);
                root = NULL;
            } else {
                Free_contact(root); 
                root = temp;        
            }
        } else {
            Contact *temp = minValueNode(root->right);
            
            Free_contact(root); 
            CopyContactData(root, temp); 
            
            root->right = deleteNode(root->right, temp->ID, manual);
        }
    }

    if (!root) return NULL;

    updateHeight(root);
    int balance = getBalance(root);

    if (balance > 1) {
        if (getBalance(root->left) >= 0) {
            return rotateRight(root);
        } else {
            root->left = rotateLeft(root->left);
            return rotateRight(root);
        }
    }
    if (balance < -1) {
        if (getBalance(root->right) <= 0) {
            return rotateLeft(root);
        } else {
            root->right = rotateRight(root->right);
            return rotateLeft(root);
        }
    }

    return root;
}

int Add_email(Contact *contact, const char *email) {
    if (contact->email_counter >= SIZE) return 0;
    contact->emails[contact->email_counter++].email = strdup(email);
    return 1;
}

int Add_phone(Contact *contact, const char *phone) {
    if (contact->phone_numbers_counter >= SIZE) return 0;
    contact->phone_numbers[contact->phone_numbers_counter++].phone_number = strdup(phone);
    return 1;
}

int Add_social(Contact *contact, const char *social) {
    if (contact->socials_counter >= SIZE) return 0;
    contact->socials[contact->socials_counter++].social = strdup(social);
    return 1;
}

void Update_person(Manual *manual, Contact *contact, 
                 const char *name, const char *last_name,
                 const char *patronymic, 
                 const char *workplace, const char *job) {

    char *new_name = strdup(name);
    char *new_last_name = strdup(last_name);
    char *new_patronymic = patronymic ? strdup(patronymic) : NULL;
    char *new_workplace = workplace ? strdup(workplace) : NULL;
    char *new_job = job ? strdup(job) : NULL;

    free(contact->person.name);
    free(contact->person.last_name);
    free(contact->person.patronymic);
    free(contact->person.workplace);
    free(contact->person.job);

    contact->person.name = new_name;
    contact->person.last_name = new_last_name;
    contact->person.patronymic = new_patronymic;
    contact->person.workplace = new_workplace;
    contact->person.job = new_job;
}

void Delete_contact_manual(Manual *manual, int id) {
    manual->root = deleteNode(manual->root, id, manual);

}

void Remove_email(Contact *contact, int index) {
    if (index < 0 || index >= contact->email_counter) return;
    free(contact->emails[index].email);
    for (int i = index; i < contact->email_counter-1; i++)
        contact->emails[i] = contact->emails[i+1];
    contact->email_counter--;
}

void Remove_phone(Contact *contact, int index) {
    if (index < 0 || index >= contact->phone_numbers_counter) return;
    free(contact->phone_numbers[index].phone_number);
    for (int i = index; i < contact->phone_numbers_counter-1; i++)
        contact->phone_numbers[i] = contact->phone_numbers[i+1];
    contact->phone_numbers_counter--;
}

void Remove_social(Contact *contact, int index) {
    if (index < 0 || index >= contact->socials_counter) return;
    free(contact->socials[index].social);
    for (int i = index; i < contact->socials_counter-1; i++)
        contact->socials[i] = contact->socials[i+1];
    contact->socials_counter--;
}

void inOrderTraversal(Contact *root, void (*func)(Contact*)) {
    if(root) {
        inOrderTraversal(root->left, func);
        func(root);
        inOrderTraversal(root->right, func);
    }
}

void Free_manual(Manual *manual) {
    inOrderTraversal(manual->root, Free_contact);
}