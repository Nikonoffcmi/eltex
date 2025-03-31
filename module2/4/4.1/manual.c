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
    contact->prev = NULL;
    contact->next = NULL;
}

void Free_contact(Contact *contact) {
    if (!contact) return;
    Free_person(&contact->person);
    
    for (int i = 0; i < contact->email_counter; i++)
        free(contact->emails[i].email);
    free(contact->emails);
    
    for (int i = 0; i < contact->phone_numbers_counter; i++)
        free(contact->phone_numbers[i].phone_number);
    free(contact->phone_numbers);
    
    for (int i = 0; i < contact->socials_counter; i++)
        free(contact->socials[i].social);
    free(contact->socials);
    
    free(contact);
}

void Init_Manual(Manual *manual) {
    manual->head = NULL;
    manual->tail = NULL;
    manual->id_counter = 0;
}

void Free_manual(Manual *manual) {
    Contact *current = manual->head;
    while (current != NULL) {
        Contact *next = current->next;
        Free_contact(current);
        current = next;
    }
    manual->head = NULL;
    manual->tail = NULL;
}

int compare_contacts(Contact *a, Contact *b) {
    int last_cmp = strcmp(a->person.last_name, b->person.last_name);
    if (last_cmp != 0) return last_cmp;
    return strcmp(a->person.name, b->person.name);
}

void Add_contact_manual(Manual *manual, const char *name, const char *last_name, 
                const char *patronymic, const char *workplace, const char *job) {
    Contact *new_contact = malloc(sizeof(Contact));
    new_contact->ID = manual->id_counter++;
    Init_person(&new_contact->person, name, last_name, patronymic, workplace, job);
    Init_contact(new_contact);
    
    if (manual->head == NULL) {
        manual->head = manual->tail = new_contact;
        return;
    }

    Contact *current = manual->head;
    while (current != NULL && compare_contacts(new_contact, current) > 0) {
        current = current->next;
    }

    if (current == manual->head) {
        new_contact->next = manual->head;
        manual->head->prev = new_contact;
        manual->head = new_contact;
    } else if (current == NULL) {
        manual->tail->next = new_contact;
        new_contact->prev = manual->tail;
        manual->tail = new_contact;
    } else {
        new_contact->prev = current->prev;
        new_contact->next = current;
        current->prev->next = new_contact;
        current->prev = new_contact;
    }
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

void Remove_contact_from_list(Manual *manual, Contact *contact) {
    if (contact->prev)
        contact->prev->next = contact->next;
    else
        manual->head = contact->next;

    if (contact->next)
        contact->next->prev = contact->prev;
    else
        manual->tail = contact->prev;

    contact->prev = NULL;
    contact->next = NULL;
}

void Reinsert_contact(Manual *manual, Contact *contact) {
    Remove_contact_from_list(manual, contact);
    
    if (manual->head == NULL) {
        manual->head = manual->tail = contact;
        return;
    }

    Contact *current = manual->head;
    while (current != NULL && compare_contacts(contact, current) > 0) {
        current = current->next;
    }

    if (current == manual->head) {
        contact->next = manual->head;
        manual->head->prev = contact;
        manual->head = contact;
    } else if (current == NULL) {
        manual->tail->next = contact;
        contact->prev = manual->tail;
        manual->tail = contact;
    } else {
        contact->prev = current->prev;
        contact->next = current;
        current->prev->next = contact;
        current->prev = contact;
    }
}

void Update_person(Manual *manual, Contact *contact, 
                 const char *name, const char *last_name,
                 const char *patronymic, 
                 const char *workplace, const char *job) {
    int name_changed = strcmp(contact->person.name, name) != 0;
    int last_name_changed = strcmp(contact->person.last_name, last_name) != 0;
    
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

    if (name_changed || last_name_changed) {
        Reinsert_contact(manual, contact);
    }
}

void Delete_contact_manual(Manual *manual, int id) {
    Contact *current = manual->head;
    while (current != NULL && current->ID != id) {
        current = current->next;
    }

    if (current == NULL) return;

    if (current->prev != NULL) {
        current->prev->next = current->next;
    } else {
        manual->head = current->next;
    }

    if (current->next != NULL) {
        current->next->prev = current->prev;
    } else {
        manual->tail = current->prev;
    }

    Free_contact(current);
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

