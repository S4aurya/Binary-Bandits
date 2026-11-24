/*
 * Hostel Mess Feedback and Waste Reduction System
 *
 * Students pre-register for the meals they will eat. The mess manager sees
 * how many students to expect, records how many plates were cooked and
 * eaten, and gets a weekly report showing which meals waste the most food.
 *
 * Files used:
 *   students.txt       id,name,room
 *   registrations.txt  date,id,breakfast,lunch,dinner   (1 = will eat, 0 = won't)
 *   meals.txt          date,meal,registered,cooked,eaten
 *   report.txt         the last weekly report
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_STUDENTS 500
#define MAX_RECORDS 2000

struct Student {
    int id;
    char name[50];
    char room[10];
};

struct MealRecord {
    char date[11];   /* YYYY-MM-DD */
    int meal;        /* 0 = breakfast, 1 = lunch, 2 = dinner */
    int registered;
    int cooked;
    int eaten;
};

struct Student students[MAX_STUDENTS];
int studentCount = 0;

struct MealRecord records[MAX_RECORDS];
int recordCount = 0;

/* reg[i][m] is 1 if student i registered for meal m on the chosen date */
int reg[MAX_STUDENTS][3];

char mealNames[3][10] = {"Breakfast", "Lunch", "Dinner"};
char dayNames[7][10] = {"Sunday", "Monday", "Tuesday", "Wednesday",
                        "Thursday", "Friday", "Saturday"};

/* ---------------- input helpers ---------------- */

void clearInput()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int readInt()
{
    int x, result;
    while ((result = scanf("%d", &x)) != 1) {
        if (result == EOF)
            exit(0);
        clearInput();
        printf("Please enter a number: ");
    }
    clearInput();
    return x;
}

void readText(char text[], int size)
{
    if (fgets(text, size, stdin) == NULL)
        exit(0);
    if (strchr(text, '\n') == NULL)
        clearInput();              /* typed more than fits: drop the rest */
    text[strcspn(text, "\n")] = '\0';
}

/* ---------------- dates (time.h) ---------------- */

void getToday(char date[])
{
    time_t now = time(NULL);
    strftime(date, 11, "%Y-%m-%d", localtime(&now));
}

/* date that is 'days' days before today */
void getDaysAgo(int days, char date[])
{
    time_t t = time(NULL) - (time_t)days * 24 * 60 * 60;
    strftime(date, 11, "%Y-%m-%d", localtime(&t));
}

/* 0 = Sunday ... 6 = Saturday */
int getWeekday(char date[])
{
    struct tm t = {0};
    int y, m, d;
    sscanf(date, "%d-%d-%d", &y, &m, &d);
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;
    t.tm_hour = 12;
    mktime(&t);
    return t.tm_wday;
}

void readDate(char date[])
{
    char line[30];
    int y, m, d;
    while (1) {
        printf("Date (YYYY-MM-DD), or press Enter for today: ");
        readText(line, 30);
        if (line[0] == '\0') {
            getToday(date);
            return;
        }
        if (sscanf(line, "%d-%d-%d", &y, &m, &d) == 3 &&
            y >= 2000 && y <= 2100 && m >= 1 && m <= 12 && d >= 1 && d <= 31) {
            sprintf(date, "%04d-%02d-%02d", y, m, d);
            return;
        }
        printf("Wrong format. Example: 2026-11-25\n");
    }
}

/* ---------------- files ---------------- */

void loadStudents()
{
    FILE *fp = fopen("students.txt", "r");
    studentCount = 0;
    if (fp == NULL)
        return;
    while (studentCount < MAX_STUDENTS &&
           fscanf(fp, "%d,%49[^,],%9s", &students[studentCount].id,
                  students[studentCount].name, students[studentCount].room) == 3)
        studentCount++;
    fclose(fp);
}

void loadMeals()
{
    FILE *fp = fopen("meals.txt", "r");
    recordCount = 0;
    if (fp == NULL)
        return;
    while (recordCount < MAX_RECORDS &&
           fscanf(fp, "%10[^,],%d,%d,%d,%d\n", records[recordCount].date,
                  &records[recordCount].meal, &records[recordCount].registered,
                  &records[recordCount].cooked, &records[recordCount].eaten) == 5)
        recordCount++;
    fclose(fp);
}

int findStudent(int id)
{
    int i;
    for (i = 0; i < studentCount; i++)
        if (students[i].id == id)
            return i;
    return -1;
}

/* Fills reg[][] for one date and counts students for each meal.
 * If a student registers twice for the same date, the later one counts. */
void countHeadcount(char date[], int count[])
{
    FILE *fp;
    char d[11];
    int id, b, l, dn, i, m;

    for (i = 0; i < MAX_STUDENTS; i++)
        for (m = 0; m < 3; m++)
            reg[i][m] = 0;

    fp = fopen("registrations.txt", "r");
    if (fp != NULL) {
        while (fscanf(fp, "%10[^,],%d,%d,%d,%d\n", d, &id, &b, &l, &dn) == 5) {
            i = findStudent(id);
            if (strcmp(d, date) == 0 && i != -1) {
                reg[i][0] = b;
                reg[i][1] = l;
                reg[i][2] = dn;
            }
        }
        fclose(fp);
    }

    for (m = 0; m < 3; m++) {
        count[m] = 0;
        for (i = 0; i < studentCount; i++)
            count[m] += reg[i][m];
    }
}

float wastePercent(int cooked, int eaten)
{
    if (cooked == 0 || eaten >= cooked)
        return 0;
    return (cooked - eaten) * 100.0 / cooked;
}

/* ---------------- menu options ---------------- */

void addStudent()
{
    struct Student s;
    FILE *fp;

    if (studentCount == MAX_STUDENTS) {
        printf("Student list is full.\n");
        return;
    }
    printf("Student ID: ");
    s.id = readInt();
    if (findStudent(s.id) != -1) {
        printf("This ID already exists.\n");
        return;
    }
    printf("Name: ");
    readText(s.name, 50);
    printf("Room: ");
    readText(s.room, 10);
    if (s.name[0] == '\0' || strchr(s.name, ',') || strchr(s.room, ',') ||
        s.room[0] == '\0' || strchr(s.room, ' ')) {
        printf("Name and room can't be empty or use commas; room can't have spaces.\n");
        return;
    }

    fp = fopen("students.txt", "a");
    fprintf(fp, "%d,%s,%s\n", s.id, s.name, s.room);
    fclose(fp);
    students[studentCount] = s;
    studentCount++;
    printf("Added %s.\n", s.name);
}

void showStudents()
{
    int i;
    if (studentCount == 0) {
        printf("No students added yet.\n");
        return;
    }
    printf("\n%-8s %-30s %s\n", "ID", "Name", "Room");
    for (i = 0; i < studentCount; i++)
        printf("%-8d %-30s %s\n", students[i].id, students[i].name, students[i].room);
}

void preRegister()
{
    int id, i, m, choice[3];
    char date[11];
    FILE *fp;

    printf("Student ID: ");
    id = readInt();
    i = findStudent(id);
    if (i == -1) {
        printf("No student with this ID. Add the student first.\n");
        return;
    }
    readDate(date);
    printf("Enter 1 if %s will eat, 0 if not.\n", students[i].name);
    for (m = 0; m < 3; m++) {
        printf("  %s: ", mealNames[m]);
        choice[m] = readInt();
        if (choice[m] != 0)
            choice[m] = 1;
    }

    fp = fopen("registrations.txt", "a");
    fprintf(fp, "%s,%d,%d,%d,%d\n", date, id, choice[0], choice[1], choice[2]);
    fclose(fp);
    printf("Registered.\n");
}

void showHeadcount()
{
    char date[11];
    int count[3], m, cook;

    readDate(date);
    countHeadcount(date, count);
    printf("\nExpected students on %s (%s)\n", date, dayNames[getWeekday(date)]);
    for (m = 0; m < 3; m++) {
        cook = (count[m] * 105 + 99) / 100;   /* 5% extra, rounded up */
        printf("  %-10s %4d registered, cook for %d\n", mealNames[m], count[m], cook);
    }
}

void recordMeal()
{
    struct MealRecord r;
    int count[3];
    FILE *fp;

    readDate(r.date);
    printf("Meal (1 Breakfast, 2 Lunch, 3 Dinner): ");
    r.meal = readInt() - 1;
    if (r.meal < 0 || r.meal > 2) {
        printf("Wrong choice.\n");
        return;
    }
    countHeadcount(r.date, count);
    r.registered = count[r.meal];
    printf("%d students had registered.\n", r.registered);
    printf("Plates cooked: ");
    r.cooked = readInt();
    printf("Plates eaten: ");
    r.eaten = readInt();
    if (r.cooked < 0 || r.eaten < 0) {
        printf("Numbers can't be negative.\n");
        return;
    }

    fp = fopen("meals.txt", "a");
    fprintf(fp, "%s,%d,%d,%d,%d\n", r.date, r.meal, r.registered, r.cooked, r.eaten);
    fclose(fp);

    if (r.eaten > r.cooked)
        printf("Food ran out: %d students more than cooked for.\n", r.eaten - r.cooked);
    else
        printf("Wasted %d plates (%.1f%%).\n", r.cooked - r.eaten,
               wastePercent(r.cooked, r.eaten));
}

/* Writes the report to report.txt, then prints the file on screen. */
void weeklyReport()
{
    char today[11], weekStart[11], line[200];
    int weekCooked[3] = {0}, weekEaten[3] = {0};
    int dayCooked[7][3] = {{0}}, dayEaten[7][3] = {{0}};
    int i, m, d, any;
    float w;
    FILE *fp;

    loadMeals();
    if (recordCount == 0) {
        printf("No meals recorded yet.\n");
        return;
    }
    getToday(today);
    getDaysAgo(6, weekStart);

    for (i = 0; i < recordCount; i++) {
        m = records[i].meal;
        d = getWeekday(records[i].date);
        dayCooked[d][m] += records[i].cooked;
        dayEaten[d][m] += records[i].eaten;
        /* dates in YYYY-MM-DD order compare correctly as text */
        if (strcmp(records[i].date, weekStart) >= 0 && strcmp(records[i].date, today) <= 0) {
            weekCooked[m] += records[i].cooked;
            weekEaten[m] += records[i].eaten;
        }
    }

    fp = fopen("report.txt", "w");
    fprintf(fp, "WEEKLY WASTE REPORT (%s to %s)\n\n", weekStart, today);
    fprintf(fp, "%-10s %8s %8s %8s\n", "Meal", "Cooked", "Eaten", "Waste");
    for (m = 0; m < 3; m++)
        fprintf(fp, "%-10s %8d %8d %7.1f%%\n", mealNames[m], weekCooked[m], weekEaten[m],
                wastePercent(weekCooked[m], weekEaten[m]));

    fprintf(fp, "\nWaste by day, all records:\n");
    fprintf(fp, "%-10s %10s %10s %10s\n", "", "Breakfast", "Lunch", "Dinner");
    for (d = 0; d < 7; d++) {
        fprintf(fp, "%-10s", dayNames[d]);
        for (m = 0; m < 3; m++) {
            if (dayCooked[d][m] == 0)
                fprintf(fp, " %10s", "-");
            else
                fprintf(fp, " %9.1f%%", wastePercent(dayCooked[d][m], dayEaten[d][m]));
        }
        fprintf(fp, "\n");
    }

    /* Cutting by (waste - 5)% still leaves about 5% extra on an average day */
    fprintf(fp, "\nSuggestions:\n");
    any = 0;
    for (d = 0; d < 7; d++)
        for (m = 0; m < 3; m++) {
            w = wastePercent(dayCooked[d][m], dayEaten[d][m]);
            if (w >= 10) {
                fprintf(fp, "  Reduce %s %s by %d%%\n", dayNames[d], mealNames[m], (int)(w - 5));
                any = 1;
            }
        }
    if (any == 0)
        fprintf(fp, "  No meal wastes more than 10%%. Keep cooking to the headcount.\n");
    fclose(fp);

    fp = fopen("report.txt", "r");
    printf("\n");
    while (fgets(line, 200, fp) != NULL)
        printf("%s", line);
    fclose(fp);
    printf("\n(Saved in report.txt)\n");
}

/* ---------------- demo ---------------- */

/* Percent of students who usually eat each meal, Sunday to Saturday */
int turnout[7][3] = {
    {40, 70, 45},   /* Sunday: many students go home */
    {65, 85, 80},
    {65, 85, 80},
    {65, 85, 80},
    {65, 85, 78},
    {60, 80, 55},   /* Friday dinner: students eat out */
    {50, 75, 60},
};

/* Simulates 4 weeks for 400 students and compares two ways of cooking:
 *   before: the cook makes food for 85% of the hostel every meal
 *   after:  the cook makes food for the registered students + 5% + 8 plates */
void demo()
{
    int hostel = 400, weeks = 4;
    int week, d, m, s, plans, registered, eaten, before, after;
    long cookedBefore = 0, cookedAfter = 0, wasteBefore = 0, wasteAfter = 0;
    long shortBefore = 0, shortAfter = 0, totalEaten = 0;
    int dayCooked[7][3] = {{0}}, dayEaten[7][3] = {{0}};
    float w;

    srand(2026);   /* fixed seed, so the demo gives the same numbers every time */

    for (week = 0; week < weeks; week++)
        for (d = 0; d < 7; d++)
            for (m = 0; m < 3; m++) {
                registered = 0;
                eaten = 0;
                for (s = 0; s < hostel; s++) {
                    plans = rand() % 100 < turnout[d][m];
                    if (plans) {
                        if (rand() % 100 < 92) {          /* 92% register */
                            registered++;
                            if (rand() % 100 >= 5)        /* 5% don't show up */
                                eaten++;
                        } else {
                            eaten++;                      /* forgot to register */
                        }
                    } else if (rand() % 100 < 3) {
                        eaten++;                          /* 3% walk in anyway */
                    }
                }

                before = hostel * 85 / 100;
                after = (registered * 105 + 99) / 100 + 8;
                totalEaten += eaten;
                cookedBefore += before;
                cookedAfter += after;

                if (before >= eaten)
                    wasteBefore += before - eaten;
                else
                    shortBefore += eaten - before;
                if (after >= eaten)
                    wasteAfter += after - eaten;
                else
                    shortAfter += eaten - after;

                dayCooked[d][m] += before;
                dayEaten[d][m] += eaten;
            }

    printf("\nDEMO: %d students, %d weeks, %d meals\n\n", hostel, weeks, weeks * 21);
    printf("%-28s %10s %14s\n", "", "Before", "Pre-register");
    printf("%-28s %10ld %14ld\n", "Plates cooked", cookedBefore, cookedAfter);
    printf("%-28s %10ld %14ld\n", "Plates eaten", totalEaten, totalEaten);
    printf("%-28s %10ld %14ld\n", "Plates wasted", wasteBefore, wasteAfter);
    printf("%-28s %9.1f%% %13.1f%%\n", "Waste", wasteBefore * 100.0 / cookedBefore,
           wasteAfter * 100.0 / cookedAfter);
    printf("%-28s %10ld %14ld\n", "Students who found no food", shortBefore, shortAfter);

    printf("\nPre-registration wastes %.0f%% fewer plates: %ld plates in %d weeks.\n",
           (wasteBefore - wasteAfter) * 100.0 / wasteBefore, wasteBefore - wasteAfter, weeks);
    printf("At 300 g per plate, that is about %.0f kg of food a month.\n",
           (wasteBefore - wasteAfter) * 0.3 / weeks * 4);

    printf("\nWithout pre-registration, the most wasted meals were:\n");
    for (d = 0; d < 7; d++)
        for (m = 0; m < 3; m++) {
            w = wastePercent(dayCooked[d][m], dayEaten[d][m]);
            if (w >= 30)
                printf("  %s %s: %.0f%% wasted\n", dayNames[d], mealNames[m], w);
        }
}

/* ---------------- main ---------------- */

int main()
{
    int choice;
    loadStudents();

    while (1) {
        printf("\n===== HOSTEL MESS =====\n");
        printf("1. Add student\n");
        printf("2. Show students\n");
        printf("3. Pre-register for meals\n");
        printf("4. Expected headcount\n");
        printf("5. Record a meal (cooked and eaten)\n");
        printf("6. Weekly waste report\n");
        printf("7. Demo: before vs after pre-registration\n");
        printf("0. Exit\n");
        printf("Choice: ");
        choice = readInt();

        if (choice == 1) addStudent();
        else if (choice == 2) showStudents();
        else if (choice == 3) preRegister();
        else if (choice == 4) showHeadcount();
        else if (choice == 5) recordMeal();
        else if (choice == 6) weeklyReport();
        else if (choice == 7) demo();
        else if (choice == 0) break;
        else printf("Wrong choice.\n");
    }
    return 0;
}
