/*
    HOSPITAL MANAGEMENT SYSTEM
    Programming Fundamentals Project - C Language
    Standard: C99

    Features:
    - Admin, Doctor and Reception menus
    - Patient and doctor management
    - Doctor appointment schedule (7 days x 8 time slots)
    - Medical history
    - Priority-based emergency queue
    - Billing with treatment and room options
    - Search, recursive binary search and sorting
    - Binary file storage
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>

#define MAX_NAME 50
#define MAX_TEXT 100
#define MAX_PHONE 20
#define DAYS 7
#define TIME_SLOTS 8
#define INITIAL_CAPACITY 5
#define MAX_FILE_RECORDS 100000U
#define DATA_FILE "hospital_data.dat"
#define TEMP_FILE "hospital_data.tmp"

/* -------------------- DATA STRUCTURES -------------------- */

struct Patient
{
    int id;
    char name[MAX_NAME];
    int age;
    char gender[10];
    char phone[MAX_PHONE];
    char address[MAX_TEXT];
    int doctor_id;                 /* -1 means no doctor assigned */
};

struct Doctor
{
    int id;
    char name[MAX_NAME];
    char specialization[MAX_TEXT];
    char phone[MAX_PHONE];
    int schedule[DAYS][TIME_SLOTS]; /* stores appointment ID; 0 means free */
};

struct MedicalHistory
{
    int patient_id;
    char illness[MAX_TEXT];
    char treatment[MAX_TEXT];
    char allergies[MAX_TEXT];
};

struct Appointment
{
    int id;
    int patient_id;
    int doctor_id;
    int day;                       /* 1 to 7 */
    int time_slot;                 /* 1 to 8 */
    int status;                    /* 1 = active, 0 = cancelled */
};

struct Emergency
{
    int patient_id;
    int priority;                  /* 5 = most urgent, 1 = least urgent */
    char condition[MAX_TEXT];
};

struct Billing
{
    int id;
    int patient_id;
    int treatment_type;
    int room_class;
    int reserved;                  /* unused; keeps older saved bill records compatible */
    float treatment_cost;
    float room_cost;
    float total_bill;
    int paid;                      /* 1 = paid, 0 = unpaid */
};

/* -------------------- GLOBAL DYNAMIC ARRAYS -------------------- */

struct Patient *patients = NULL;
struct Doctor *doctors = NULL;
struct MedicalHistory *histories = NULL;
struct Appointment *appointments = NULL;
struct Emergency *emergencyQueue = NULL;
struct Billing *bills = NULL;

int patientCount = 0, patientCapacity = 0;
int doctorCount = 0, doctorCapacity = 0;
int historyCount = 0, historyCapacity = 0;
int appointmentCount = 0, appointmentCapacity = 0;
int emergencyCount = 0, emergencyCapacity = 0;
int billCount = 0, billCapacity = 0;

/* -------------------- FUNCTION PROTOTYPES -------------------- */

/* General input and memory */
int read_int(const char message[]);
float read_float(const char message[]);
void read_line(const char message[], char input[], int size);
int ensure_capacity(void **array, int *capacity, int needed, size_t itemSize);
void free_memory(void);

/* Main and role menus */
void main_menu(void);
void admin_menu(void);
void doctor_menu(void);
void reception_menu(void);
void patient_menu(void);
void doctor_management_menu(void);
void appointment_menu(void);
void history_menu(void);
void emergency_menu(void);
void billing_menu(void);

/* ID and lookup helpers */
int find_patient_index(int id);
int find_doctor_index(int id);
int find_appointment_index(int id);
int next_appointment_id(void);
int next_bill_id(void);

/* Patient management */
void add_patient(void);
void edit_patient(void);
void delete_patient(void);
void view_patients(void);
void search_patient(void);
void assign_doctor_to_patient(void);
int binary_search_patient(int id, int left, int right);
void bubble_sort_patients(void);

/* Doctor management */
void add_doctor(void);
void edit_doctor(void);
void view_doctors(void);
void selection_sort_doctors(void);

/* Appointments */
void book_appointment(void);
void cancel_appointment(void);
void view_appointments(void);
void view_doctor_appointments(int doctor_id);
void display_schedule(int doctor_index);

/* Medical history */
void add_medical_history(void);
void view_medical_history(void);
void view_patient_history(int patient_id);

/* Emergency queue */
void add_emergency(void);
void view_emergency_queue(void);

/* Billing */
void generate_bill(void);
void view_bills(void);
void mark_bill_paid(void);

/* File handling */
void save_data(void);
void load_data(void);

/* -------------------- GENERAL INPUT FUNCTIONS -------------------- */

/* Reads an integer and rejects invalid text. */
int read_int(const char message[])
{
    char input[100];
    char *end;
    long value;

    while (1)
    {
        printf("%s", message);

        if (fgets(input, sizeof(input), stdin) == NULL)
            return 0;

        value = strtol(input, &end, 10);

        while (isspace((unsigned char)*end))
            end++;

        if (end != input && *end == '\0' && value >= INT_MIN && value <= INT_MAX)
            return (int)value;

        printf("Invalid input. Please enter a whole number.\n");
    }
}

/* Reads a decimal number for bill-related values. */
float read_float(const char message[])
{
    char input[100];
    char *end;
    float value;

    while (1)
    {
        printf("%s", message);

        if (fgets(input, sizeof(input), stdin) == NULL)
            return 0.0f;

        value = strtof(input, &end);

        while (isspace((unsigned char)*end))
            end++;

        if (end != input && *end == '\0')
            return value;

        printf("Invalid input. Please enter a number.\n");
    }
}

/* Reads a complete line, including spaces. */
void read_line(const char message[], char input[], int size)
{
    size_t length;

    printf("%s", message);

    if (fgets(input, size, stdin) == NULL)
    {
        input[0] = '\0';
        return;
    }

    length = strlen(input);

    if (length > 0 && input[length - 1] == '\n')
    {
        input[length - 1] = '\0';
    }
    else
    {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
        {
            /* Discard characters that do not fit in the input array. */
        }
    }
}

/* Expands a dynamic array when more records are required. */
int ensure_capacity(void **array, int *capacity, int needed, size_t itemSize)
{
    int newCapacity;
    void *temporary;

    if (needed <= *capacity)
        return 1;

    newCapacity = (*capacity == 0) ? INITIAL_CAPACITY : *capacity;

    while (newCapacity < needed)
        newCapacity *= 2;

    temporary = realloc(*array, (size_t)newCapacity * itemSize);

    if (temporary == NULL)
    {
        printf("Memory allocation failed.\n");
        return 0;
    }

    *array = temporary;
    *capacity = newCapacity;
    return 1;
}

/* Frees every dynamic array before the program closes. */
void free_memory(void)
{
    free(patients);
    free(doctors);
    free(histories);
    free(appointments);
    free(emergencyQueue);
    free(bills);

    patients = NULL;
    doctors = NULL;
    histories = NULL;
    appointments = NULL;
    emergencyQueue = NULL;
    bills = NULL;

    patientCount = doctorCount = historyCount = 0;
    appointmentCount = emergencyCount = billCount = 0;
    patientCapacity = doctorCapacity = historyCapacity = 0;
    appointmentCapacity = emergencyCapacity = billCapacity = 0;
}

/* -------------------- LOOKUP AND ID FUNCTIONS -------------------- */

/* Returns the array position of a patient, or -1 if not found. */
int find_patient_index(int id)
{
    int i;

    for (i = 0; i < patientCount; i++)
    {
        if (patients[i].id == id)
            return i;
    }

    return -1;
}

/* Returns the array position of a doctor, or -1 if not found. */
int find_doctor_index(int id)
{
    int i;

    for (i = 0; i < doctorCount; i++)
    {
        if (doctors[i].id == id)
            return i;
    }

    return -1;
}

/* Returns the array position of an appointment, or -1 if not found. */
int find_appointment_index(int id)
{
    int i;

    for (i = 0; i < appointmentCount; i++)
    {
        if (appointments[i].id == id)
            return i;
    }

    return -1;
}

/* Generates the next appointment ID. */
int next_appointment_id(void)
{
    int i, highest = 0;

    for (i = 0; i < appointmentCount; i++)
        if (appointments[i].id > highest)
            highest = appointments[i].id;

    return highest + 1;
}

/* Generates the next bill ID. */
int next_bill_id(void)
{
    int i, highest = 0;

    for (i = 0; i < billCount; i++)
        if (bills[i].id > highest)
            highest = bills[i].id;

    return highest + 1;
}

/* -------------------- PATIENT MANAGEMENT -------------------- */

/* Registers a new patient and rejects duplicate patient IDs. */
void add_patient(void)
{
    struct Patient newPatient;
    int doctorID;

    printf("\n========== REGISTER PATIENT ==========\n");

    newPatient.id = read_int("Enter Patient ID: ");

    if (newPatient.id <= 0)
    {
        printf("Patient ID must be positive.\n");
        return;
    }

    if (find_patient_index(newPatient.id) != -1)
    {
        printf("This Patient ID already exists.\n");
        return;
    }

    read_line("Enter Patient Name: ", newPatient.name, MAX_NAME);
    if (newPatient.name[0] == '\0')
    {
        printf("Name cannot be empty.\n");
        return;
    }

    newPatient.age = read_int("Enter Age: ");
    if (newPatient.age < 1 || newPatient.age > 120)
    {
        printf("Invalid age.\n");
        return;
    }

    read_line("Enter Gender: ", newPatient.gender, sizeof(newPatient.gender));
    read_line("Enter Phone Number: ", newPatient.phone, MAX_PHONE);
    read_line("Enter Address: ", newPatient.address, MAX_TEXT);

    newPatient.doctor_id = -1;

    if (doctorCount > 0)
    {
        doctorID = read_int("Enter Doctor ID to assign (0 for none): ");

        if (doctorID != 0)
        {
            if (find_doctor_index(doctorID) == -1)
            {
                printf("Doctor not found. Patient will remain unassigned.\n");
            }
            else
            {
                newPatient.doctor_id = doctorID;
            }
        }
    }

    if (!ensure_capacity((void **)&patients, &patientCapacity,
                         patientCount + 1, sizeof(struct Patient)))
        return;

    patients[patientCount] = newPatient;
    patientCount++;

    printf("Patient registered successfully.\n");
}

/* Updates the details of an existing patient. */
void edit_patient(void)
{
    int id, index, newAge;

    printf("\n========== EDIT PATIENT ==========\n");
    id = read_int("Enter Patient ID: ");
    index = find_patient_index(id);

    if (index == -1)
    {
        printf("Patient not found.\n");
        return;
    }

    printf("Current name: %s\n", patients[index].name);
    read_line("Enter New Name: ", patients[index].name, MAX_NAME);

    newAge = read_int("Enter New Age: ");
    if (newAge < 1 || newAge > 120)
    {
        printf("Invalid age. Existing age was kept.\n");
    }
    else
    {
        patients[index].age = newAge;
    }

    read_line("Enter New Gender: ", patients[index].gender, sizeof(patients[index].gender));
    read_line("Enter New Phone: ", patients[index].phone, MAX_PHONE);
    read_line("Enter New Address: ", patients[index].address, MAX_TEXT);

    printf("Patient information updated.\n");
}

/* Removes a patient and clears records linked to that patient. */
void delete_patient(void)
{
    int id, index, i, j;

    printf("\n========== DELETE PATIENT ==========\n");
    id = read_int("Enter Patient ID: ");
    index = find_patient_index(id);

    if (index == -1)
    {
        printf("Patient not found.\n");
        return;
    }

    printf("Deleting patient: %s\n", patients[index].name);

    /* Cancel active appointments and free their doctor schedule slots. */
    for (i = 0; i < appointmentCount; i++)
    {
        if (appointments[i].patient_id == id && appointments[i].status == 1)
        {
            int doctorIndex = find_doctor_index(appointments[i].doctor_id);
            if (doctorIndex != -1)
                doctors[doctorIndex].schedule[appointments[i].day - 1]
                                             [appointments[i].time_slot - 1] = 0;
            appointments[i].status = 0;
        }
    }

    /* Remove the patient's medical history records. */
    i = 0;
    while (i < historyCount)
    {
        if (histories[i].patient_id == id)
        {
            for (j = i; j < historyCount - 1; j++)
                histories[j] = histories[j + 1];
            historyCount--;
        }
        else i++;
    }

    /* Remove the patient from the emergency queue. */
    i = 0;
    while (i < emergencyCount)
    {
        if (emergencyQueue[i].patient_id == id)
        {
            for (j = i; j < emergencyCount - 1; j++)
                emergencyQueue[j] = emergencyQueue[j + 1];
            emergencyCount--;
        }
        else i++;
    }

    /* Remove the patient's bills. */
    i = 0;
    while (i < billCount)
    {
        if (bills[i].patient_id == id)
        {
            for (j = i; j < billCount - 1; j++)
                bills[j] = bills[j + 1];
            billCount--;
        }
        else i++;
    }

    /* Remove the patient record itself. */
    for (i = index; i < patientCount - 1; i++)
        patients[i] = patients[i + 1];

    patientCount--;
    printf("Patient and linked records deleted.\n");
}

/* Displays every registered patient. */
void view_patients(void)
{
    int i;

    printf("\n========== PATIENT RECORDS ==========\n");

    if (patientCount == 0)
    {
        printf("No patient records found.\n");
        return;
    }

    for (i = 0; i < patientCount; i++)
    {
        printf("\nID: %d | Name: %s | Age: %d\n",
               patients[i].id, patients[i].name, patients[i].age);
        printf("Gender: %s | Phone: %s\n",
               patients[i].gender, patients[i].phone);
        printf("Address: %s\n", patients[i].address);

        if (patients[i].doctor_id == -1)
            printf("Assigned Doctor: None\n");
        else
            printf("Assigned Doctor ID: %d\n", patients[i].doctor_id);
    }
}

/* Sorts patient records by ID using Bubble Sort. */
void bubble_sort_patients(void)
{
    int i, j;
    struct Patient temporary;

    for (i = 0; i < patientCount - 1; i++)
    {
        for (j = 0; j < patientCount - i - 1; j++)
        {
            if (patients[j].id > patients[j + 1].id)
            {
                temporary = patients[j];
                patients[j] = patients[j + 1];
                patients[j + 1] = temporary;
            }
        }
    }
}

/* Recursive Binary Search; patient records must be sorted by ID first. */
int binary_search_patient(int id, int left, int right)
{
    int middle;

    if (left > right)
        return -1;

    middle = left + (right - left) / 2;

    if (patients[middle].id == id)
        return middle;

    if (id < patients[middle].id)
        return binary_search_patient(id, left, middle - 1);

    return binary_search_patient(id, middle + 1, right);
}

/* Searches for a patient by ID using recursive Binary Search. */
void search_patient(void)
{
    int id, index;

    if (patientCount == 0)
    {
        printf("No patient records available.\n");
        return;
    }

    id = read_int("Enter Patient ID to search: ");
    bubble_sort_patients();
    index = binary_search_patient(id, 0, patientCount - 1);

    if (index == -1)
        printf("Patient not found.\n");
    else
    {
        printf("\nPatient found: %s\n", patients[index].name);
        printf("ID: %d | Age: %d | Phone: %s\n",
               patients[index].id, patients[index].age, patients[index].phone);
        printf("Address: %s\n", patients[index].address);
    }
}

/* Assigns an existing doctor to an existing patient. */
void assign_doctor_to_patient(void)
{
    int patientID, doctorID, patientIndex;

    patientID = read_int("Enter Patient ID: ");
    patientIndex = find_patient_index(patientID);

    if (patientIndex == -1)
    {
        printf("Patient not found.\n");
        return;
    }

    doctorID = read_int("Enter Doctor ID (0 to remove assignment): ");

    if (doctorID == 0)
    {
        patients[patientIndex].doctor_id = -1;
        printf("Doctor assignment removed.\n");
    }
    else if (find_doctor_index(doctorID) == -1)
    {
        printf("Doctor not found. Assignment unchanged.\n");
    }
    else
    {
        patients[patientIndex].doctor_id = doctorID;
        printf("Doctor assigned successfully.\n");
    }
}

/* -------------------- DOCTOR MANAGEMENT -------------------- */

/* Registers a doctor with an initially empty appointment schedule. */
void add_doctor(void)
{
    struct Doctor newDoctor;
    int i, j;

    printf("\n========== ADD DOCTOR ==========\n");
    newDoctor.id = read_int("Enter Doctor ID: ");

    if (newDoctor.id <= 0 || find_doctor_index(newDoctor.id) != -1)
    {
        printf("Invalid or duplicate Doctor ID.\n");
        return;
    }

    read_line("Enter Doctor Name: ", newDoctor.name, MAX_NAME);
    read_line("Enter Specialization: ", newDoctor.specialization, MAX_TEXT);
    read_line("Enter Phone Number: ", newDoctor.phone, MAX_PHONE);

    for (i = 0; i < DAYS; i++)
        for (j = 0; j < TIME_SLOTS; j++)
            newDoctor.schedule[i][j] = 0;

    if (!ensure_capacity((void **)&doctors, &doctorCapacity,
                         doctorCount + 1, sizeof(struct Doctor)))
        return;

    doctors[doctorCount++] = newDoctor;
    printf("Doctor added successfully.\n");
}

/* Updates a doctor's profile. */
void edit_doctor(void)
{
    int id, index;

    id = read_int("Enter Doctor ID to edit: ");
    index = find_doctor_index(id);

    if (index == -1)
    {
        printf("Doctor not found.\n");
        return;
    }

    read_line("Enter New Name: ", doctors[index].name, MAX_NAME);
    read_line("Enter New Specialization: ", doctors[index].specialization, MAX_TEXT);
    read_line("Enter New Phone: ", doctors[index].phone, MAX_PHONE);

    printf("Doctor profile updated.\n");
}

/* Sorts doctors alphabetically by name using Selection Sort. */
void selection_sort_doctors(void)
{
    int i, j, minimum;
    struct Doctor temporary;

    for (i = 0; i < doctorCount - 1; i++)
    {
        minimum = i;

        for (j = i + 1; j < doctorCount; j++)
        {
            if (strcmp(doctors[j].name, doctors[minimum].name) < 0)
                minimum = j;
        }

        if (minimum != i)
        {
            temporary = doctors[i];
            doctors[i] = doctors[minimum];
            doctors[minimum] = temporary;
        }
    }
}

/* Displays all doctors and their specializations. */
void view_doctors(void)
{
    int i;

    if (doctorCount == 0)
    {
        printf("No doctors registered.\n");
        return;
    }

    selection_sort_doctors();
    printf("\n========== DOCTOR LIST ==========\n");

    for (i = 0; i < doctorCount; i++)
    {
        printf("ID: %d | Name: %s | Specialization: %s | Phone: %s\n",
               doctors[i].id, doctors[i].name,
               doctors[i].specialization, doctors[i].phone);
    }
}

/* -------------------- APPOINTMENT MANAGEMENT -------------------- */

/* Displays a doctor's weekly schedule. */
void display_schedule(int doctor_index)
{
    int day, slot;

    printf("\nSchedule for Dr. %s\n", doctors[doctor_index].name);
    printf("A = booked, - = available\n");

    for (day = 0; day < DAYS; day++)
    {
        printf("Day %d: ", day + 1);
        for (slot = 0; slot < TIME_SLOTS; slot++)
        {
            if (doctors[doctor_index].schedule[day][slot] == 0)
                printf("[%d:-] ", slot + 1);
            else
                printf("[%d:A] ", slot + 1);
        }
        printf("\n");
    }
}

/* Books an appointment and prevents a doctor from having two bookings in one slot. */
void book_appointment(void)
{
    int patientID, doctorID, patientIndex, doctorIndex;
    int day, slot, appointmentID;
    struct Appointment newAppointment;

    printf("\n========== BOOK APPOINTMENT ==========\n");
    patientID = read_int("Enter Patient ID: ");
    patientIndex = find_patient_index(patientID);

    if (patientIndex == -1)
    {
        printf("Patient not found. Register the patient first.\n");
        return;
    }

    doctorID = read_int("Enter Doctor ID: ");
    doctorIndex = find_doctor_index(doctorID);

    if (doctorIndex == -1)
    {
        printf("Doctor not found.\n");
        return;
    }

    display_schedule(doctorIndex);
    day = read_int("Choose day (1-7): ");
    slot = read_int("Choose time slot (1-8): ");

    if (day < 1 || day > DAYS || slot < 1 || slot > TIME_SLOTS)
    {
        printf("Invalid day or time slot.\n");
        return;
    }

    if (doctors[doctorIndex].schedule[day - 1][slot - 1] != 0)
    {
        printf("This doctor is already booked in that slot. Choose another slot.\n");
        return;
    }

    if (!ensure_capacity((void **)&appointments, &appointmentCapacity,
                         appointmentCount + 1, sizeof(struct Appointment)))
        return;

    appointmentID = next_appointment_id();

    newAppointment.id = appointmentID;
    newAppointment.patient_id = patientID;
    newAppointment.doctor_id = doctorID;
    newAppointment.day = day;
    newAppointment.time_slot = slot;
    newAppointment.status = 1;

    appointments[appointmentCount++] = newAppointment;
    doctors[doctorIndex].schedule[day - 1][slot - 1] = appointmentID;
    patients[patientIndex].doctor_id = doctorID;

    printf("Appointment booked. Appointment ID: %d\n", appointmentID);
}

/* Cancels an appointment and frees the matching schedule slot. */
void cancel_appointment(void)
{
    int id, index, doctorIndex;

    id = read_int("Enter Appointment ID to cancel: ");
    index = find_appointment_index(id);

    if (index == -1 || appointments[index].status == 0)
    {
        printf("Active appointment not found.\n");
        return;
    }

    doctorIndex = find_doctor_index(appointments[index].doctor_id);

    if (doctorIndex != -1)
    {
        doctors[doctorIndex].schedule[appointments[index].day - 1]
                                     [appointments[index].time_slot - 1] = 0;
    }

    appointments[index].status = 0;
    printf("Appointment cancelled successfully.\n");
}

/* Displays all appointments, including their active/cancelled status. */
void view_appointments(void)
{
    int i;

    if (appointmentCount == 0)
    {
        printf("No appointment records.\n");
        return;
    }

    printf("\n========== APPOINTMENT LIST ==========\n");

    for (i = 0; i < appointmentCount; i++)
    {
        printf("Appointment %d | Patient %d | Doctor %d | Day %d | Slot %d | %s\n",
               appointments[i].id, appointments[i].patient_id,
               appointments[i].doctor_id, appointments[i].day,
               appointments[i].time_slot,
               appointments[i].status ? "Active" : "Cancelled");
    }
}

/* Displays appointments belonging to one logged-in doctor. */
void view_doctor_appointments(int doctor_id)
{
    int i, found = 0;

    printf("\n========== YOUR APPOINTMENTS ==========\n");

    for (i = 0; i < appointmentCount; i++)
    {
        if (appointments[i].doctor_id == doctor_id && appointments[i].status == 1)
        {
            printf("Appointment %d | Patient ID %d | Day %d | Slot %d\n",
                   appointments[i].id, appointments[i].patient_id,
                   appointments[i].day, appointments[i].time_slot);
            found = 1;
        }
    }

    if (!found)
        printf("No active appointments for this doctor.\n");
}

/* -------------------- MEDICAL HISTORY -------------------- */

/* Adds a new medical-history entry for an existing patient. */
void add_medical_history(void)
{
    struct MedicalHistory newHistory;

    newHistory.patient_id = read_int("Enter Patient ID: ");

    if (find_patient_index(newHistory.patient_id) == -1)
    {
        printf("Patient not found.\n");
        return;
    }

    read_line("Previous illness / diagnosis: ", newHistory.illness, MAX_TEXT);
    read_line("Treatment given: ", newHistory.treatment, MAX_TEXT);
    read_line("Allergies (write None if not known): ", newHistory.allergies, MAX_TEXT);

    if (!ensure_capacity((void **)&histories, &historyCapacity,
                         historyCount + 1, sizeof(struct MedicalHistory)))
        return;

    histories[historyCount++] = newHistory;
    printf("Medical history saved.\n");
}

/* Displays all medical-history entries for one patient. */
void view_patient_history(int patient_id)
{
    int i, found = 0;

    for (i = 0; i < historyCount; i++)
    {
        if (histories[i].patient_id == patient_id)
        {
            printf("Illness: %s\nTreatment: %s\nAllergies: %s\n---\n",
                   histories[i].illness, histories[i].treatment,
                   histories[i].allergies);
            found = 1;
        }
    }

    if (!found)
        printf("No medical history recorded for this patient.\n");
}

/* Prompts for a patient ID and displays that patient's history. */
void view_medical_history(void)
{
    int id = read_int("Enter Patient ID: ");

    if (find_patient_index(id) == -1)
    {
        printf("Patient not found.\n");
        return;
    }

    printf("\nMedical history for Patient %d:\n", id);
    view_patient_history(id);
}

/* -------------------- EMERGENCY PRIORITY QUEUE -------------------- */

/* Adds emergency patients in descending order of urgency (5 is highest). */
void add_emergency(void)
{
    struct Emergency newEmergency;
    int i, position;

    newEmergency.patient_id = read_int("Enter Patient ID: ");

    if (find_patient_index(newEmergency.patient_id) == -1)
    {
        printf("Patient not found. Register the patient first.\n");
        return;
    }

    for (i = 0; i < emergencyCount; i++)
    {
        if (emergencyQueue[i].patient_id == newEmergency.patient_id)
        {
            printf("This patient is already in the emergency queue.\n");
            return;
        }
    }

    newEmergency.priority = read_int("Urgency (5 = critical, 1 = less urgent): ");

    if (newEmergency.priority < 1 || newEmergency.priority > 5)
    {
        printf("Priority must be from 1 to 5.\n");
        return;
    }

    read_line("Describe the emergency condition: ", newEmergency.condition, MAX_TEXT);

    if (!ensure_capacity((void **)&emergencyQueue, &emergencyCapacity,
                         emergencyCount + 1, sizeof(struct Emergency)))
        return;

    /* Find the correct position so the highest priority stays at the front. */
    position = emergencyCount;
    while (position > 0 &&
           emergencyQueue[position - 1].priority < newEmergency.priority)
    {
        emergencyQueue[position] = emergencyQueue[position - 1];
        position--;
    }

    emergencyQueue[position] = newEmergency;
    emergencyCount++;

    printf("Patient added to the emergency queue.\n");
}

/* Displays emergency patients from highest to lowest urgency. */
void view_emergency_queue(void)
{
    int i;

    printf("\n========== EMERGENCY QUEUE ==========\n");

    if (emergencyCount == 0)
    {
        printf("Emergency queue is empty.\n");
        return;
    }

    for (i = 0; i < emergencyCount; i++)
    {
        printf("Priority %d | Patient ID %d | %s\n",
               emergencyQueue[i].priority,
               emergencyQueue[i].patient_id,
               emergencyQueue[i].condition);
    }
}

/* -------------------- BILLING -------------------- */

/* Creates a bill using treatment and room charges. */
void generate_bill(void)
{
    struct Billing newBill;
    int patientID, choice;

    printf("\n========== GENERATE BILL ==========\n");
    patientID = read_int("Enter Patient ID: ");

    if (find_patient_index(patientID) == -1)
    {
        printf("Patient not found.\n");
        return;
    }

    newBill.patient_id = patientID;

    printf("\nTreatment Type\n");
    printf("1. Consultation       (Rs. 1500)\n");
    printf("2. Laboratory Tests   (Rs. 3000)\n");
    printf("3. Surgery            (Rs. 25000)\n");
    printf("4. Emergency Treatment(Rs. 8000)\n");
    choice = read_int("Choose treatment: ");
    newBill.treatment_type = choice;

    switch (choice)
    {
        case 1: newBill.treatment_cost = 1500.0f; break;
        case 2: newBill.treatment_cost = 3000.0f; break;
        case 3: newBill.treatment_cost = 25000.0f; break;
        case 4: newBill.treatment_cost = 8000.0f; break;
        default:
            printf("Invalid treatment choice.\n");
            return;
    }

    printf("\nRoom Class\n");
    printf("1. General Ward (Rs. 1000)\n");
    printf("2. Semi-Private (Rs. 3000)\n");
    printf("3. Private Room (Rs. 6000)\n");
    printf("4. No Room Charge (Rs. 0)\n");
    choice = read_int("Choose room class: ");
    newBill.room_class = choice;

    switch (choice)
    {
        case 1: newBill.room_cost = 1000.0f; break;
        case 2: newBill.room_cost = 3000.0f; break;
        case 3: newBill.room_cost = 6000.0f; break;
        case 4: newBill.room_cost = 0.0f; break;
        default:
            printf("Invalid room choice.\n");
            return;
    }

    newBill.total_bill = newBill.treatment_cost + newBill.room_cost;
    newBill.id = next_bill_id();
    newBill.paid = 0;

    if (!ensure_capacity((void **)&bills, &billCapacity,
                         billCount + 1, sizeof(struct Billing)))
        return;

    bills[billCount++] = newBill;

    printf("\nBill generated successfully.\n");
    printf("Bill ID: %d\n", newBill.id);
    printf("Treatment cost: Rs. %.2f\n", newBill.treatment_cost);
    printf("Room cost: Rs. %.2f\n", newBill.room_cost);
    printf("Total payable: Rs. %.2f\n", newBill.total_bill);
}

/* Displays all bills. */
void view_bills(void)
{
    int i;

    if (billCount == 0)
    {
        printf("No billing records.\n");
        return;
    }

    printf("\n========== BILLING RECORDS ==========\n");

    for (i = 0; i < billCount; i++)
    {
        printf("Bill ID: %d | Patient ID: %d | Total: Rs. %.2f | %s\n",
               bills[i].id, bills[i].patient_id, bills[i].total_bill,
               bills[i].paid ? "Paid" : "Unpaid");
    }
}

/* Marks an existing bill as paid. */
void mark_bill_paid(void)
{
    int id, i;

    id = read_int("Enter Bill ID: ");

    for (i = 0; i < billCount; i++)
    {
        if (bills[i].id == id)
        {
            bills[i].paid = 1;
            printf("Bill marked as paid.\n");
            return;
        }
    }

    printf("Bill not found.\n");
}

/* -------------------- SUBMENUS -------------------- */

/* Patient options. */
void patient_menu(void)
{
    int choice;

    do
    {
        printf("\n========== PATIENT MANAGEMENT ==========\n");
        printf("1. Register Patient\n2. Edit Patient\n3. Delete Patient\n");
        printf("4. View All Patients\n5. Search by ID\n");
        printf("6. Assign / Change Doctor\n0. Back\n");
        choice = read_int("Enter choice: ");

        switch (choice)
        {
            case 1: add_patient(); break;
            case 2: edit_patient(); break;
            case 3: delete_patient(); break;
            case 4: view_patients(); break;
            case 5: search_patient(); break;
            case 6: assign_doctor_to_patient(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* Doctor profile options. */
void doctor_management_menu(void)
{
    int choice;

    do
    {
        printf("\n========== DOCTOR MANAGEMENT ==========\n");
        printf("1. Add Doctor\n2. Edit Doctor\n3. View Doctors\n0. Back\n");
        choice = read_int("Enter choice: ");

        switch (choice)
        {
            case 1: add_doctor(); break;
            case 2: edit_doctor(); break;
            case 3: view_doctors(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* Appointment options. */
void appointment_menu(void)
{
    int choice;

    do
    {
        printf("\n========== APPOINTMENT MANAGEMENT ==========\n");
        printf("1. Book Appointment\n2. Cancel Appointment\n3. View Appointments\n0. Back\n");
        choice = read_int("Enter choice: ");

        switch (choice)
        {
            case 1: book_appointment(); break;
            case 2: cancel_appointment(); break;
            case 3: view_appointments(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* Medical history options. */
void history_menu(void)
{
    int choice;

    do
    {
        printf("\n========== MEDICAL HISTORY ==========\n");
        printf("1. Add History / Treatment\n2. View Patient History\n0. Back\n");
        choice = read_int("Enter choice: ");

        switch (choice)
        {
            case 1: add_medical_history(); break;
            case 2: view_medical_history(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* Emergency queue options. */
void emergency_menu(void)
{
    int choice;

    do
    {
        printf("\n========== EMERGENCY QUEUE ==========\n");
        printf("1. Register Emergency Patient\n2. View Queue\n0. Back\n");
        choice = read_int("Enter choice: ");

        switch (choice)
        {
            case 1: add_emergency(); break;
            case 2: view_emergency_queue(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* Billing options. */
void billing_menu(void)
{
    int choice;

    do
    {
        printf("\n========== BILLING ==========\n");
        printf("1. Generate Bill\n2. View Bills\n3. Mark Bill as Paid\n0. Back\n");
        choice = read_int("Enter choice: ");

        switch (choice)
        {
            case 1: generate_bill(); break;
            case 2: view_bills(); break;
            case 3: mark_bill_paid(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* -------------------- ROLE MENUS -------------------- */

/* Admin can access all modules. */
void admin_menu(void)
{
    int choice;

    do
    {
        printf("\n========== ADMIN MENU ==========\n");
        printf("1. Patient Management\n2. Doctor Management\n3. Appointments\n");
        printf("4. Medical History\n5. Emergency Queue\n");
        printf("0. Back to Main Menu\n");
        choice = read_int("Enter choice: ");

        switch (choice)
        {
            case 1: patient_menu(); break;
            case 2: doctor_management_menu(); break;
            case 3: appointment_menu(); break;
            case 4: history_menu(); break;
            case 5: emergency_menu(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* Doctor enters their registered ID to see their own appointments and patient history. */
void doctor_menu(void)
{
    int doctorID, choice;

    doctorID = read_int("Enter your Doctor ID: ");

    if (find_doctor_index(doctorID) == -1)
    {
        printf("Doctor ID not found.\n");
        return;
    }

    do
    {
        printf("\n========== DOCTOR MENU (ID %d) ==========\n", doctorID);
        printf("1. View My Appointments\n2. View Patient Medical History\n");
        printf("3. View Emergency Queue\n0. Back\n");
        choice = read_int("Enter choice: ");

        switch (choice)
        {
            case 1: view_doctor_appointments(doctorID); break;
            case 2: view_medical_history(); break;
            case 3: view_emergency_queue(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* Reception can register patients, manage appointments, emergencies and bills. */
void reception_menu(void)
{
    int choice;

    do
    {
        printf("\n========== RECEPTION MENU ==========\n");
        printf("1. Patient Management\n2. Book Appointment\n3. Cancel Appointment\n");
        printf("4. Register Emergency Patient\n5. Billing\n0. Back\n");
        choice = read_int("Enter choice: ");

        switch (choice)
        {
            case 1: patient_menu(); break;
            case 2: book_appointment(); break;
            case 3: cancel_appointment(); break;
            case 4: add_emergency(); break;
            case 5: billing_menu(); break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* -------------------- FILE STORAGE -------------------- */

struct FileHeader
{
    char magic[8];
    unsigned int version;
    unsigned int counts[6];
};

/* Saves every module to one binary data file. */
void save_data(void)
{
    FILE *file;
    struct FileHeader header;
    int success = 1;

    file = fopen(TEMP_FILE, "wb");

    if (file == NULL)
    {
        printf("Could not create data file.\n");
        return;
    }

    strcpy(header.magic, "HMSDATA");
    header.version = 1;
    header.counts[0] = (unsigned int)patientCount;
    header.counts[1] = (unsigned int)doctorCount;
    header.counts[2] = (unsigned int)historyCount;
    header.counts[3] = (unsigned int)appointmentCount;
    header.counts[4] = (unsigned int)emergencyCount;
    header.counts[5] = (unsigned int)billCount;

    if (fwrite(&header, sizeof(header), 1, file) != 1)
        success = 0;

    if (success && patientCount > 0 && fwrite(patients, sizeof(struct Patient), patientCount, file) != (size_t)patientCount)
        success = 0;
    if (success && doctorCount > 0 && fwrite(doctors, sizeof(struct Doctor), doctorCount, file) != (size_t)doctorCount)
        success = 0;
    if (success && historyCount > 0 && fwrite(histories, sizeof(struct MedicalHistory), historyCount, file) != (size_t)historyCount)
        success = 0;
    if (success && appointmentCount > 0 && fwrite(appointments, sizeof(struct Appointment), appointmentCount, file) != (size_t)appointmentCount)
        success = 0;
    if (success && emergencyCount > 0 && fwrite(emergencyQueue, sizeof(struct Emergency), emergencyCount, file) != (size_t)emergencyCount)
        success = 0;
    if (success && billCount > 0 && fwrite(bills, sizeof(struct Billing), billCount, file) != (size_t)billCount)
        success = 0;

    if (fclose(file) != 0)
        success = 0;

    if (!success)
    {
        remove(TEMP_FILE);
        printf("Error while saving data. Existing data file was not replaced.\n");
        return;
    }

    /* Remove old destination first for compatibility with Windows compilers. */
    remove(DATA_FILE);

    if (rename(TEMP_FILE, DATA_FILE) != 0)
    {
        printf("Could not finalize the data file.\n");
        return;
    }

    printf("All data saved successfully in %s.\n", DATA_FILE);
}

/* Loads saved records; missing or invalid files are handled without crashing. */
void load_data(void)
{
    FILE *file;
    struct FileHeader header;
    int valid = 1;
    int i, j;
    int doctorIndex;

    file = fopen(DATA_FILE, "rb");

    if (file == NULL)
    {
        printf("No previous data file found. Starting with empty records.\n");
        return;
    }

    if (fread(&header, sizeof(header), 1, file) != 1)
        valid = 0;

    if (valid && (strcmp(header.magic, "HMSDATA") != 0 || header.version != 1))
        valid = 0;

    for (i = 0; valid && i < 6; i++)
    {
        if (header.counts[i] > MAX_FILE_RECORDS)
            valid = 0;
    }

    if (valid && header.counts[0] > 0 &&
        ensure_capacity((void **)&patients, &patientCapacity,
                        (int)header.counts[0], sizeof(struct Patient)))
    {
        if (fread(patients, sizeof(struct Patient), header.counts[0], file) != header.counts[0])
            valid = 0;
    }
    else if (valid && header.counts[0] > 0) valid = 0;

    if (valid && header.counts[1] > 0 &&
        ensure_capacity((void **)&doctors, &doctorCapacity,
                        (int)header.counts[1], sizeof(struct Doctor)))
    {
        if (fread(doctors, sizeof(struct Doctor), header.counts[1], file) != header.counts[1])
            valid = 0;
    }
    else if (valid && header.counts[1] > 0) valid = 0;

    if (valid && header.counts[2] > 0 &&
        ensure_capacity((void **)&histories, &historyCapacity,
                        (int)header.counts[2], sizeof(struct MedicalHistory)))
    {
        if (fread(histories, sizeof(struct MedicalHistory), header.counts[2], file) != header.counts[2])
            valid = 0;
    }
    else if (valid && header.counts[2] > 0) valid = 0;

    if (valid && header.counts[3] > 0 &&
        ensure_capacity((void **)&appointments, &appointmentCapacity,
                        (int)header.counts[3], sizeof(struct Appointment)))
    {
        if (fread(appointments, sizeof(struct Appointment), header.counts[3], file) != header.counts[3])
            valid = 0;
    }
    else if (valid && header.counts[3] > 0) valid = 0;

    if (valid && header.counts[4] > 0 &&
        ensure_capacity((void **)&emergencyQueue, &emergencyCapacity,
                        (int)header.counts[4], sizeof(struct Emergency)))
    {
        if (fread(emergencyQueue, sizeof(struct Emergency), header.counts[4], file) != header.counts[4])
            valid = 0;
    }
    else if (valid && header.counts[4] > 0) valid = 0;

    if (valid && header.counts[5] > 0 &&
        ensure_capacity((void **)&bills, &billCapacity,
                        (int)header.counts[5], sizeof(struct Billing)))
    {
        if (fread(bills, sizeof(struct Billing), header.counts[5], file) != header.counts[5])
            valid = 0;
    }
    else if (valid && header.counts[5] > 0) valid = 0;

    /* Reject trailing bytes as well as truncated files. */
    if (valid && fgetc(file) != EOF)
        valid = 0;

    fclose(file);

    if (!valid)
    {
        free_memory();
        printf("Warning: data file is damaged or incompatible. Starting with empty records.\n");
        return;
    }

    patientCount = (int)header.counts[0];
    doctorCount = (int)header.counts[1];
    historyCount = (int)header.counts[2];
    appointmentCount = (int)header.counts[3];
    emergencyCount = (int)header.counts[4];
    billCount = (int)header.counts[5];

    /* Rebuild each doctor's 2D schedule from active appointment records. */
    for (i = 0; i < doctorCount; i++)
        for (j = 0; j < DAYS; j++)
        {
            int slot;
            for (slot = 0; slot < TIME_SLOTS; slot++)
                doctors[i].schedule[j][slot] = 0;
        }

    for (i = 0; i < appointmentCount; i++)
    {
        if (appointments[i].status == 1 &&
            appointments[i].day >= 1 && appointments[i].day <= DAYS &&
            appointments[i].time_slot >= 1 && appointments[i].time_slot <= TIME_SLOTS)
        {
            doctorIndex = find_doctor_index(appointments[i].doctor_id);
            if (doctorIndex != -1)
            {
                int d = appointments[i].day - 1;
                int s = appointments[i].time_slot - 1;
                if (doctors[doctorIndex].schedule[d][s] == 0)
                    doctors[doctorIndex].schedule[d][s] = appointments[i].id;
                else
                    appointments[i].status = 0; /* Resolve conflicting saved bookings safely. */
            }
            else
                appointments[i].status = 0;
        }
        else
            appointments[i].status = 0;
    }

    /* Restore emergency queue priority order after loading. */
    for (i = 0; i < emergencyCount - 1; i++)
    {
        int k, best = i;
        struct Emergency temporary;
        for (k = i + 1; k < emergencyCount; k++)
            if (emergencyQueue[k].priority > emergencyQueue[best].priority)
                best = k;
        if (best != i)
        {
            temporary = emergencyQueue[i];
            emergencyQueue[i] = emergencyQueue[best];
            emergencyQueue[best] = temporary;
        }
    }

    printf("Saved data loaded successfully.\n");
}

/* -------------------- MAIN MENU AND PROGRAM ENTRY -------------------- */

/* Main menu and automatic save before exit. */
void main_menu(void)
{
    int choice;

    do
    {
        printf("\n========================================\n");
        printf("       HOSPITAL MANAGEMENT SYSTEM       \n");
        printf("========================================\n");
        printf("1. Admin Login\n");
        printf("2. Doctor Login\n");
        printf("3. Reception\n");
        printf("4. Save Data Now\n");
        printf("0. Save and Exit\n");

        choice = read_int("Enter choice: ");

        switch (choice)
        {
            case 1: admin_menu(); break;
            case 2: doctor_menu(); break;
            case 3: reception_menu(); break;
            case 4: save_data(); break;
            case 0:
                save_data();
                printf("Thank you for using the Hospital Management System.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);
}

int main(void)
{
    printf("Loading Hospital Management System...\n");
    load_data();
    main_menu();
    free_memory();
    return 0;
}
