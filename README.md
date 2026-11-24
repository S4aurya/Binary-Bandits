# Hostel Mess Feedback and Waste Reduction System

## Problem

The mess cooks about the same amount of food every day because it doesn't know how many students
will come. On Sundays many students go home, so a lot of food is thrown away.

## Solution

Students tell the mess in advance which meals they will eat. The mess cooks for that number plus a
little extra. After each meal, the manager enters how many plates were cooked and eaten, and the
program works out the waste and suggests which meals to cut down.

## How to run

```
gcc mess.c -o mess
./mess
```

On Windows, run `mess.exe`.

## Menu

1. **Add student:** ID, name and room, saved in `students.txt`
2. **Show students**
3. **Pre-register for meals:** a student picks breakfast, lunch and dinner for a date, saved in
   `registrations.txt`
4. **Expected headcount:** how many students registered for each meal, and how much to cook
   (5% extra)
5. **Record a meal:** plates cooked and plates eaten, saved in `meals.txt`, and the waste %
6. **Weekly waste report:** this week's waste, a table of waste for each day and meal, and
   suggestions like "Reduce Sunday Dinner by 35%", saved in `report.txt`
7. **Demo:** compares cooking the same amount every day with cooking for pre-registered students

## Demo result

The demo simulates 400 students for 4 weeks.

| | Before | With pre-registration |
|---|---|---|
| Plates cooked | 28,560 | 23,364 |
| Plates wasted | 5,915 | 723 |
| Waste | 20.7% | 3.1% |
| Students who found no food | 11 | 15 |

Pre-registration wastes **88% fewer plates**, about 1,500 kg of food a month. These numbers come
from a simulation, not a real mess.

## C concepts used

- **Structures:** `struct Student`, `struct MealRecord`
- **2D arrays:** `reg[student][meal]`, `turnout[day][meal]`, `dayCooked[day][meal]`
- **File handling:** `fopen`, `fprintf`, `fscanf`, `fgets`, append and write modes
- **Statistics:** waste percentage, totals per meal and per day, suggestions
- **time.h:** today's date, the day of the week, the date 6 days ago
