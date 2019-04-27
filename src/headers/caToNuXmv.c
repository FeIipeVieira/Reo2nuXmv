#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"

void nullPorts(struct Automato *automato, struct Transition *transition, FILE *f)
{
    int exists = 0;
    struct StringList *portsAutomato = automato->ports;
    struct StringList *portsTrans = transition->ports;
    while (portsAutomato != NULL)
    {
        if (!existString(portsTrans, portsAutomato->string))
        {
            fprintf(f, "ports.%s = NULL & ", portsAutomato->string);
        }
        portsAutomato = portsAutomato->nextString;
    }
}

void caToNuxmv(struct Automato *automato, FILE *f)
{
    int nStates = automato->nStates;
    struct StateList *states = automato->states;
    fprintf(f, "MODULE %s(time)\nVAR\n\tports: ", automato->name);
    fprintf(f, "portsModule\n");
    fprintf(f, "\tcs: {");
    while (states != NULL)
    {
        fprintf(f, "%s%s", states->state->name, states->nextState != NULL ? "," : "");
        states = states->nextState;
    }
    states = automato->states;
    fprintf(f, "};\n");
    fprintf(f, "TRANS\n\t");
    struct TransitionList *transitions;
    struct ConditionList *conditions;
    char operation[2];
    operation[1] = '\0';
    while (states != NULL)
    {
        transitions = states->state->transitions;
        while (transitions != NULL)
        {
            fprintf(f, "((cs = %s & ", transitions->transition->start->name);
            nullPorts(automato, transitions->transition, f);
            fprintf(f, "%s)-> next(cs) = %s)%s", transitions->transition->condition,
                    transitions->transition->end->name, (transitions->nextTransition != NULL || states->nextState != NULL) ? " & " : ";\n\n");
            transitions = transitions->nextTransition;
        }
        states = states->nextState;
    }
}

struct StringList *portsIntersection(struct Transition *transition, struct Automato *automato)
{
    struct StringList *list = NULL;
    char *concat = (char *)malloc(600 * sizeof(char));
    struct StringList *ports = transition->ports;
    struct StringList *portsAutomato = automato->ports;
    while (ports != NULL)
    {
        portsAutomato = automato->ports;
        while (portsAutomato != NULL)
        {
            if (strcmp(ports->string, portsAutomato->string) == 0)
            {
                strcpy(concat, portsAutomato->string);
                list = addString(list, concat);
            }
            portsAutomato = portsAutomato->nextString;
        }
        ports = ports->nextString;
    }
    return list;
}

int equalIntersections(struct StringList *list1, struct StringList *list2)
{
    if (list1 == NULL && list2 != NULL)
    {
        return 0;
    }
    int equal = 1;
    int matches = 0;
    struct StringList *list1Start = list1;
    struct StringList *list2Start = list2;
    int list1Len = 0;
    int list2Len = 0;
    int firtsPass = 1;
    while (list1 != NULL)
    {
        list2 = list2Start;
        while (list2 != NULL)
        {
            if (list1Len == 0)
            {
                list2Len++;
            }
            if (strcmp(list1->string, list2->string) == 0)
            {
                matches = 1;
                if (list1Len != 0)
                {
                    break;
                }
            }
            list2 = list2->nextString;
        }
        list1Len++;
        if (!matches)
            return 0;
        matches = 0;
        list1 = list1->nextString;
    }
    if (list2Len > list1Len)
    {
        list1 = list1Start;
        list2 = list2Start;
        while (list2 != NULL)
        {
            list1 = list1Start;
            while (list1 != NULL)
            {
                if (strcmp(list2->string, list1->string) == 0)
                {
                    matches = 1;
                    break;
                }
                list1 = list1->nextString;
            }
            if (!matches)
                return 0;
            matches = 0;
            list2 = list2->nextString;
        }
    }
    return 1;
}

int equalPorts(struct Transition *transition1, struct Transition *transition2)
{
    int equal = 0;
    struct StringList *biggerConditions;
    struct StringList *lesserConditions;
    struct StringList *tempLesserConditions;
    if (transition1->nPorts > transition2->nPorts)
    {
        biggerConditions = transition1->ports;
        lesserConditions = transition2->ports;
    }
    else
    {
        biggerConditions = transition2->ports;
        lesserConditions = transition1->ports;
    }
    tempLesserConditions = lesserConditions;
    while (biggerConditions != NULL)
    {
        lesserConditions = tempLesserConditions;
        while (lesserConditions != NULL)
        {
            if (strcmp(biggerConditions->string, lesserConditions->string) == 0)
            {
                equal = 1;
                break;
            }
            lesserConditions = lesserConditions->nextString;
        }
        if (!equal)
            return 0;
        equal = 0;
        biggerConditions = biggerConditions->nextString;
    }
    return 1;
}

void printToNuXmv(struct StringList *trans, struct StringList *states, struct StringList *invar,
                  struct StringList *components, FILE *f)
{
    char *concat = (char *)malloc(600 * sizeof(char));
    snprintf(concat, 600, "%s%s", components->string, components->nextString->string);
    fprintf(f, "MODULE %s(time)\nVAR\n", concat);
    fprintf(f, "\tprod1: %s(time);\n\tprod2: %s(time);\n\tports: portsModule;\n",
            components->string, components->nextString->string);
    fprintf(f, "\tcs: {");
    while (states != NULL)
    {
        fprintf(f, "%s%s", states->string, states->nextString != NULL ? "," : "");
        states = states->nextString;
    }
    fprintf(f, "};\n");
    if (trans != NULL)
    {
        fprintf(f, "TRANS\n");
        while (trans != NULL)
        {
            fprintf(f, "\t%s%s", trans->string, trans->nextString != NULL ? " &\n" : ";\n");
            trans = trans->nextString;
        }
    }
    fprintf(f, "INVAR\n");
    while (invar != NULL)
    {
        fprintf(f, "\t%s%s", invar->string, invar->nextString != NULL ? " &\n" : ";\n\n");
        invar = invar->nextString;
    }
}

void portsToNuXmv(FILE *f, struct Automato *automato)
{
    struct StringList *ports = automato->ports;
    int timeMax = 3;
    fprintf(f, "MODULE portsModule\nFROZENVAR\n");
    while (ports != NULL)
    {
        fprintf(f, "\t%s : array 0..%d of{NULL, 0, 1};\n", ports->string, timeMax);
        ports = ports->nextString;
    }
    fprintf(f, "ASSIGN\n");
    ports = automato->ports;
    while (ports != NULL)
    {
        for (size_t i = 0; i < timeMax; i++)
        {
            fprintf(f, "\tinit(%s[%ld]) := 0;\n", ports->string, i);
        }
        ports = ports->nextString;
    }
}

void printaAutomatoFinal(struct Automato *automato)
{
    FILE *f = fopen("nuxmv2.smv", "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    fprintf(f, "MODULE main\nVAR\n\ttime: 0..3;\n\tautomato: %s(time);\n", automato->name);
    fprintf(f, "ASSIGN\n\tinit(time) := 0;\n\tnext(time) := case\n\t\ttime < 3: time + 1;\n\t\tTRUE: time;\nesac;\n\n");
    caToNuxmv(automato, f);
    portsToNuXmv(f, automato);

    fclose(f);
}

char *nullPortsToString(struct StringList *portsAutomato, struct StringList *ports)
{
    char *nullPortsString = (char *)malloc(600 * sizeof(char));
    char *nullPort = (char *)malloc(600 * sizeof(char));
    while (portsAutomato != NULL)
    {
        if (!existString(ports, portsAutomato->string))
        {
            snprintf(nullPort, 600, " & ports.%s = NULL", portsAutomato->string);
            strcat(nullPortsString, nullPort);
        }
        portsAutomato = portsAutomato->nextString;
    }
    return nullPortsString;
}

struct Automato *productInSmv(struct AutomatoList *automatos, FILE *f)
{
    struct Automato *automato1 = automatos->automato;
    struct Automato *automato2;
    char *concat = (char *)malloc(600 * sizeof(char));
    char *tempCondition = (char *)malloc(600 * sizeof(char));
    char *transString = (char *)malloc(600 * sizeof(char));
    char *portString = (char *)malloc(6000 * sizeof(char));
    struct StringList *intersection1 = NULL;
    struct StringList *intersection2 = NULL;
    struct StateList *states1;
    struct StateList *states2;
    struct TransitionList *transitions1 = NULL;
    struct TransitionList *transitions2 = NULL;
    struct StringList *trans = NULL;
    struct StringList *invar = NULL;
    struct StringList *states = NULL;
    struct StringList *components = NULL;
    struct StringList *inalcStates = NULL;
    struct StringList *automatoPorts = NULL;
    struct ConditionList *conditions;
    struct State *tempState = NULL;
    struct State *stateStart = NULL;
    struct StateList *tempStates = NULL;
    struct Transition *tempTransition = NULL;
    struct StringList *tempPorts = NULL;
    int tempNPorts;
    int firstPass = 1;

    while (automatos->nextAutomato != NULL)
    {
        automato2 = automatos->nextAutomato->automato;
        states1 = automato1->states;
        components = NULL;
        components = addString(components, automato1->name);
        components = addString(components, automato2->name);
        automatoPorts = unionStringList(automato1->ports, automato2->ports);
        while (states1 != NULL)
        {
            states2 = automato2->states;
            invar = NULL;
            states = NULL;
            while (states2 != NULL)
            {
                snprintf(concat, 600, "%s%s", states1->state->name, states2->state->name);
                states = addString(states, concat);
                snprintf(transString, 600, "(((prod1.cs = %s) & (prod2.cs = %s)) <-> (cs = %s)", states1->state->name, states2->state->name, concat);
                invar = addString(invar, transString);
                states2 = states2->nextState;
                tempState = newState(concat);
                tempStates = addStateToList(tempStates, tempState);
            }
            states1 = states1->nextState;
        }
        states1 = automato1->states;
        states2 = automato2->states;
        trans = NULL;
        while (states1 != NULL)
        {
            states2 = automato2->states;
            while (states2 != NULL)
            {
                snprintf(concat, 600, "%s%s", states1->state->name, states2->state->name);
                stateStart = findState(tempStates, concat);
                transitions1 = states1->state->transitions;
                delStringList(inalcStates);
                inalcStates = cpyStringList(inalcStates, states);
                snprintf(concat, 600, "%s%s", states1->state->name, states2->state->name);
                inalcStates = delString(inalcStates, concat);
                firstPass = 1;
                while (transitions1 != NULL)
                {
                    intersection1 = portsIntersection(transitions1->transition, automato2);
                    if (intersection1 == NULL)
                    {
                        snprintf(concat, 600, "%s%s", transitions1->transition->end->name, states2->state->name);
                        inalcStates = delString(inalcStates, concat);
                        tempState = findState(tempStates, concat);
                        tempTransition = (struct Transition *)malloc(sizeof(struct Transition));
                        tempTransition->start = stateStart;
                        tempTransition->end = tempState;
                        tempTransition->nPorts = transitions1->transition->nPorts;
                        tempTransition->ports = transitions1->transition->ports;
                        tempTransition->condition = transitions1->transition->condition;
                        addTransition(tempTransition);
                    }
                    transitions2 = states2->state->transitions;
                    while (transitions2 != NULL)
                    {
                        intersection2 = portsIntersection(transitions2->transition, automato1);
                        if (intersection2 == NULL && firstPass)
                        {
                            snprintf(concat, 600, "%s%s", states1->state->name, transitions2->transition->end->name);
                            inalcStates = delString(inalcStates, concat);
                            tempState = findState(tempStates, concat);
                            tempTransition = (struct Transition *)malloc(sizeof(struct Transition));
                            tempTransition->start = stateStart;
                            tempTransition->end = tempState;
                            tempTransition->nPorts = transitions2->transition->nPorts;
                            tempTransition->ports = transitions2->transition->ports;
                            tempTransition->condition = transitions2->transition->condition;
                            addTransition(tempTransition);
                        }
                        if (equalIntersections(intersection1, intersection2))
                        {
                            snprintf(concat, 600, "%s%s", transitions1->transition->end->name, transitions2->transition->end->name);
                            inalcStates = delString(inalcStates, concat);
                            tempNPorts = transitions1->transition->nPorts;
                            tempPorts = transitions1->transition->ports;
                            if (!equalPorts(transitions1->transition, transitions2->transition))
                            {
                                strcpy(portString, "");
                                strcpy(transString, "");
                                tempPorts = unionStringList(transitions1->transition->ports, transitions2->transition->ports);
                                snprintf(transString, 600, "((cs = %s%s) & (%s & %s)", states1->state->name, states2->state->name,
                                         transitions1->transition->condition, transitions2->transition->condition);
                                portString = nullPortsToString(automatoPorts, tempPorts);
                                strcat(transString, portString);
                                snprintf(portString, 600, " -> next(cs) = %s%s)", transitions1->transition->end->name, transitions2->transition->end->name);
                                strcat(transString, portString);
                                trans = addString(trans, transString);
                                tempNPorts = listLength(tempPorts);
                            }
                            tempState = findState(tempStates, concat);
                            tempTransition = (struct Transition *)malloc(sizeof(struct Transition));
                            tempTransition->start = stateStart;
                            tempTransition->end = tempState;
                            tempTransition->nPorts = tempNPorts;
                            tempTransition->ports = tempPorts;
                            tempCondition = (char *)malloc(600 * sizeof(char));
                            snprintf(tempCondition, 600, "%s & %s", transitions1->transition->condition, transitions2->transition->condition);
                            tempTransition->condition = tempCondition;
                            addTransition(tempTransition);
                        }
                        transitions2 = transitions2->nextTransition;
                    }
                    firstPass = 0;
                    transitions1 = transitions1->nextTransition;
                }
                if (states1->state->transitions == NULL)
                {
                    transitions2 = states2->state->transitions;
                    while (transitions2 != NULL)
                    {
                        intersection2 = portsIntersection(transitions2->transition, automato1);
                        if (intersection2 == NULL)
                        {
                            snprintf(concat, 600, "%s%s", states1->state->name, transitions2->transition->end->name);
                            inalcStates = delString(inalcStates, concat);
                            tempState = findState(tempStates, concat);
                            tempTransition = (struct Transition *)malloc(sizeof(struct Transition));
                            tempTransition->start = stateStart;
                            tempTransition->end = tempState;
                            tempTransition->nPorts = transitions2->transition->nPorts;
                            tempTransition->ports = transitions2->transition->ports;
                            tempTransition->condition = transitions2->transition->condition;
                            addTransition(tempTransition);
                        }
                        transitions2 = transitions2->nextTransition;
                    }
                }
                if (inalcStates != NULL)
                {
                    snprintf(transString, 600, "((cs = %s%s) -> (", states1->state->name, states2->state->name);
                    while (inalcStates != NULL)
                    {
                        snprintf(concat, 600, "(next(cs) != %s)%s", inalcStates->string, inalcStates->nextString != NULL ? " & " : ")");
                        strcat(transString, concat);
                        inalcStates = inalcStates->nextString;
                    }
                    trans = addString(trans, transString);
                }
                states2 = states2->nextState;
            }
            states1 = states1->nextState;
        }
        printToNuXmv(trans, states, invar, components, f);
        automatos = automatos->nextAutomato;
        snprintf(concat, 600, "%s%s", components->string, components->nextString->string);
        automato1 = newAutomato(concat);
        while (tempStates != NULL)
        {
            addState(tempStates->state, automato1);
            tempStates = tempStates->nextState;
        }
        tempStates = NULL;
    }
    return automato1;
}

void startNuxmv(struct AutomatoList *automatos)
{
    struct AutomatoList *automatoList = automatos;
    struct Automato *finalAutomato;
    FILE *f = fopen("nuxmv.smv", "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    fprintf(f, "MODULE main\nVAR\n\ttime: 0..3;\n\t%s: %s(time);\n", "prod", "prod");
    fprintf(f, "ASSIGN\n\tinit(time) := 0;\n\tnext(time) := case\n\t\ttime < 3: time + 1;\n\t\tTRUE: time;\nesac;\n\n");
    while (automatoList != NULL)
    {
        caToNuxmv(automatoList->automato, f);
        automatoList = automatoList->nextAutomato;
    }
    finalAutomato = productInSmv(automatos, f);
    printaAutomatoFinal(finalAutomato);
    portsToNuXmv(f, finalAutomato);

    fclose(f);
}