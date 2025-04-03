#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "menu.h"

void Clear_input_buffer() {
    while(getchar() != '\n');
}

void Print_contact(Contact *c) {
    printf("\nID: %d", c->ID);
    printf("\nФ.И.О.: %s %s%s%s", 
            c->person.last_name,
            c->person.name,
            c->person.patronymic ? " " : "",
            c->person.patronymic ? c->person.patronymic : "");
    printf("\nМесто работы: %s\nДолжность: %s", c->person.workplace ? c->person.workplace : "-", 
            c->person.job ? c->person.job : "-");
    
    printf("\nПочта (%d):", c->email_counter);
    for(int i = 0; i < c->email_counter; i++)
        printf("\n  %d. %s", i+1, c->emails[i].email);
    
    printf("\nНомера телефонов (%d):", c->phone_numbers_counter);
    for(int i = 0; i < c->phone_numbers_counter; i++)
        printf("\n  %d. %s", i+1, c->phone_numbers[i].phone_number);
    
    printf("\nСоцсети (%d):", c->socials_counter);
    for(int i = 0; i < c->socials_counter; i++)
        printf("\n  %d. %s", i+1, c->socials[i].social);
    printf("\n");
}

void Display_contacts(Manual *manual) {
    if(manual->root == NULL) {
        printf("\nКонтактов не найдено!\n");
        return;
    }
    
    printf("\n--- Список Контактов ---\n");
    inOrderTraversal(manual->root, Print_contact);
    printf("\n");
}

void Add_contact_menu(Manual *manual) {
    char name[50], last_name[50], patronymic[100], workplace[100], job[100];
    
    printf("\n--- Добавить контакт ---\n");
    printf("Имя: "); 
    scanf("%49s", name);
    
    printf("Фамилия: "); 
    scanf("%49s", last_name);

    printf("Отчество (необязательно): ");
    Clear_input_buffer();
    fgets(patronymic, 100, stdin);
    patronymic[strcspn(patronymic, "\n")] = 0;

    printf("Место работы (необязательно): "); 
    fgets(workplace, 100, stdin);
    workplace[strcspn(workplace, "\n")] = 0;

    printf("Должность (необязательно): ");
    fgets(job, 100, stdin);
    job[strcspn(job, "\n")] = 0;
    
    Add_contact_manual(manual, name, last_name, 
                *patronymic ? patronymic : NULL,
                *workplace ? workplace : NULL,
                *job ? job : NULL);
    
    Contact *new = findContactById(manual->root, manual->id_counter - 1);
    
    while(1) {
        printf("\nДобавить другие данные:\n");
        printf("1. Почта\n2. Номер телефона\n3. Соцсеть\n4. Закончить\nВыбрать пункт: ");
        int choice;
        scanf("%d", &choice);
        
        if(choice == 4) break;
        
        char data[100];
        printf("Введите значение: ");
        scanf("%99s", data);
        
        switch(choice) {
            case 1: Add_email(new, data); break;
            case 2: Add_phone(new, data); break;
            case 3: Add_social(new, data); break;
            default: printf("Неверно выбран пункт!\n");
        }
    }
}

void printTree(Contact *root, int level, int is_right) {
    if (!root) return;
    
    static char* lines[1024];
    static int last_level = -1;
    
    printTree(root->right, level + 1, 1);

    for(int i = 0; i < level; i++) {
        if(i == level-1) {
            printf(is_right ? " ┌──" : " └──");
        }
        else {
            if(last_level >= i && lines[i]) 
                printf(" │  ");
            else 
                printf("    ");
        }
    }
    
    printf("(%d) %s %s\n", root->ID, 
           root->person.last_name, 
           root->person.name);
    
    last_level = level;
    
    printTree(root->left, level + 1, 0);
}

void displayTree(Manual *manual) {
    printf("\n--- Дерево ---\n");
    printTree(manual->root, 0, 0);
    printf("\n");
}

void Edit_contact_menu(Manual *manual) {
    Display_contacts(manual);
    if(manual->root == NULL) return;
    
    int id;
    printf("Введите ID контакта для изменений: ");
    scanf("%d", &id);
    
    Contact *target = findContactById(manual->root, id);
    
    if(!target) {
        printf("Контакт не найден!\n");
        return;
    }
    
    while(1) {
        Print_contact(target);
        printf("\n--- Меню изменений ---\n");
        printf("1. Изменить Ф.И.О.\n2. Изменить почту\n3. Изменить номер телефона\n");
        printf("4. Изменить соцсеть\n5. Изменить сведения о роботе\n6. Сохранить и выйти\nВыбрать пункт: ");
        
        int choice;
        scanf("%d", &choice);
        
        if(choice == 6) break;
        
        char data[100];
        int index;
        switch(choice) {
            case 1: {
                char name[50], last_name[50], patronymic[100];
                printf("Новое имя: "); 
                scanf("%49s", name);
                
                printf("Новая Фамилия: "); 
                scanf("%49s", last_name);
                
                printf("Отчество (необязательно): ");
                Clear_input_buffer();
                fgets(patronymic, 100, stdin);
                patronymic[strcspn(patronymic, "\n")] = 0;

                Update_person(manual, target, name, last_name, 
                            *patronymic ? patronymic : NULL,
                            target->person.workplace, 
                            target->person.job);
                break;
            }
            case 2: {
                printf("1. Добавить почту\n2. Удалить почту\n3. Выйти\nВыбрать пункт: ");
                scanf("%d", &choice);
                if(choice == 1) {
                    printf("Почта: "); scanf("%99s", data);
                    Add_email(target, data);
                } else if (choice == 2){
                    printf("Введите индекс почты для удаления: ");
                    scanf("%d", &index);
                    Remove_email(target, index-1);
                }
                break;
            }
            case 3: {
                printf("1. Добавить номер телефона\n2. Удалить номер телефона\n3. Выйти\nВыбрать пункт: ");
                scanf("%d", &choice);
                if(choice == 1) {
                    printf("Номер телефона: "); scanf("%99s", data);
                    Add_phone(target, data);
                } else if (choice == 2 ){
                    printf("Введите индекс телефона для удаления: ");
                    scanf("%d", &index);
                    Remove_phone(target, index-1);
                }
                break;
            }
            case 4: {
                printf("1. Добавить соцсеть\n2. Удалить соцсеть\n3. Выйти\nВыбрать пункт: ");
                scanf("%d", &choice);
                if(choice == 1) {
                    printf("Соцсеть: "); scanf("%99s", data);
                    Add_social(target, data);
                } else if (choice == 2){
                    printf("Введите индекс соцсети для удаления: ");
                    scanf("%d", &index);
                    Remove_social(target, index-1);
                }
                break;
            }
            case 5: {
                Clear_input_buffer();
                char new_workplace[100], new_job[100];
                
                printf("Новое место роботы (текущее: %s)\nВведите новое или '-' для удаления: ",
                        target->person.workplace ? target->person.workplace : "-");
                fgets(new_workplace, 100, stdin);
                new_workplace[strcspn(new_workplace, "\n")] = 0;
                
                printf("Новая должность (текущая: %s)\nВведите новое или '-' для удаления: ",
                        target->person.job ? target->person.job : "-");
                fgets(new_job, 100, stdin);
                new_job[strcspn(new_job, "\n")] = 0;
                
                char *wp = (*new_workplace && strcmp(new_workplace, "-")) ? new_workplace : NULL;
                char *jb = (*new_job && strcmp(new_job, "-")) ? new_job : NULL;
                
                Update_person(manual, target, 
                            target->person.name,
                            target->person.last_name,
                            target->person.patronymic,
                            wp,
                            jb);
                break;
            }
            default: printf("Неверно выбран пункт!\n");
        }
    }
}

void Menu() {
    Manual contacts;
    Init_Manual(&contacts);
    
    while(1) {
        printf("\n--- Справочник контактов ---\n");
        printf("1. Добавить контакт\n2. Посмотреть все контакты\n3. Изменить контакт\n");
        printf("4. Удалить контакт\n5. Выйти\nВыбрать пункт: ");
        
        int choice;
        scanf("%d", &choice);
        
        switch(choice) {
            case 1: 
                Add_contact_menu(&contacts);
                break;
            case 2: 
                // Display_contacts(&contacts);
                displayTree(&contacts);
                break;
            case 3: 
                Edit_contact_menu(&contacts);
                break;
            case 4: {
                Display_contacts(&contacts);
                if(contacts.id_counter == 0) break;
                
                int id;
                printf("Введите ID контакта для удаления: ");
                scanf("%d", &id);
                Delete_contact_manual(&contacts, id);
                break;
            }
            case 5: 
                Free_manual(&contacts);
                exit(0);
            default: 
                printf("Неверно выбран контакт!\n");
        }
    }
}