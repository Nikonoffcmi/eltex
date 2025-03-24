#define SIZE 16

typedef struct {
    char *name;
    char *last_name;
    char *patronymic;
    char *job;
    char *workplace;
} Person;

typedef struct
{
    char *phone_number;
} Phone_number;

typedef struct
{
    char* email;
} Email;

typedef struct
{
    char *social;
} Social;

typedef struct
{
    int ID;
    Person person;
    Email *emails;
    Phone_number *phone_numbers;
    Social *socials;
    int email_counter;
    int phone_numbers_counter;
    int socials_counter;
} Contact;

typedef struct {
    Contact *list;
    int id;
    int users;
    int capacity;
} Manual;


void Init_person(Person *p, const char *name, const char *last_name, 
                const char *patronymic, const char *workplace, const char *job);
void Init_contact(Contact *contact);
void Init_Manual(Manual *manual);

void Add_contact_manual(Manual *manual, const char *name, const char *last_name, 
                const char *patronymic, const char *workplace, const char *job);
int Add_email(Contact *contact, const char *email);
int Add_phone(Contact *contact, const char *phone);
int Add_social(Contact *contact, const char *social);

void Update_person(Contact *contact, const char *name, const char *last_name, 
                 const char *patronymic, const char *workplace, const char *job);

void Delete_contact_manual(Manual *manual, int id);
void Remove_email(Contact *contact, int index);
void Remove_phone(Contact *contact, int index);
void Remove_social(Contact *contact, int index);

void Free_person(Person *p);
void Free_contact(Contact *contact);
void Free_manual(Manual *manual);