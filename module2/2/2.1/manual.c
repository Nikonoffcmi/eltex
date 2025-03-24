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
}

void Init_Manual(Manual *manual) {
    manual->capacity = SIZE;
    manual->list = malloc(manual->capacity * sizeof(Contact));
    manual->id = 0;
    manual->users = 0;
}

void Free_manual(Manual *manual) {
    for (int i = 0; i < manual->users; i++) {
        Free_contact(&manual->list[i]);
    }
    free(manual->list);
}

void Add_contact_manual(Manual *manual, const char *name, const char *last_name, 
                const char *patronymic, const char *workplace, const char *job) {
    if (manual->users >= manual->capacity) {
        manual->capacity *= 2;
        manual->list = realloc(manual->list, manual->capacity * sizeof(Contact));
    }
    
    Contact *new = &manual->list[manual->users];
    new->ID = manual->id++;
    Init_person(&new->person, name, last_name, patronymic, workplace, job);
    Init_contact(new);
    manual->users++;
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

void Update_person(Contact *contact, const char *name, const char *last_name, 
                 const char *patronymic, const char *workplace, const char *job) {
    char *new_name = strdup(name);
    char *new_last_name = strdup(last_name);
    char *new_patronymic = patronymic ? strdup(patronymic) : NULL;
    char *new_workplace = workplace ? strdup(workplace) : NULL;
    char *new_job = job ? strdup(job) : NULL;
    
    Free_person(&contact->person);
    
    contact->person.name = new_name;
    contact->person.last_name = new_last_name;
    contact->person.patronymic = new_patronymic;
    contact->person.workplace = new_workplace;
    contact->person.job = new_job;
}

void Delete_contact_manual(Manual *manual, int id) {
    int index = -1;
    for (int i = 0; i < manual->users; i++)
        if (manual->list[i].ID == id) {
            index = i;
            break;
        }
    if (index == -1) return;
    
    Free_contact(&manual->list[index]);
    if (index != manual->users-1)
        manual->list[index] = manual->list[manual->users-1];
    manual->users--;
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
