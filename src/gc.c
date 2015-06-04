/*
 * Copyright (C) Tildeslash Ltd. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 *
 * You must obey the GNU Affero General Public License in all respects
 * for all of the code used other than OpenSSL.
 */

#include "config.h"

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

// libmonit
#include "util/List.h"

#include "monit.h"
#include "protocol.h"
#include "process.h"
#include "engine.h"


/* Private prototypes */
static void _gc_service_list(Service_T *);
static void _gc_service(Service_T *);
static void _gc_servicegroup(ServiceGroup_T *);
static void _gc_mail_server(MailServer_T *);
static void _gcportlist(Port_T *);
static void _gcfilesystem(Filesystem_T *);
static void _gcicmp(Icmp_T *);
static void _gcpql(Resource_T *);
static void _gcptl(Timestamp_T *);
static void _gcparl(ActionRate_T *);
static void _gc_action(Action_T *);
static void _gc_eventaction(EventAction_T *);
static void _gcpdl(Dependant_T *);
static void _gcso(Size_T *);
static void _gclinkstatus(LinkStatus_T *);
static void _gclinkspeed(LinkSpeed_T *);
static void _gclinksaturation(LinkSaturation_T *);
static void _gcbandwidth(Bandwidth_T *);
static void _gcmatch(Match_T *);
static void _gcchecksum(Checksum_T *);
static void _gcperm(Perm_T *);
static void _gcstatus(Status_T *);
static void _gcuid(Uid_T *);
static void _gcgid(Gid_T *);
static void _gcpid(Pid_T *);
static void _gcppid(Pid_T *);
static void _gcfsflag(Fsflag_T *);
static void _gcnonexist(Nonexist_T *);
static void _gcgeneric(Generic_T *);
static void _gcath(Auth_T *);
static void _gc_mmonit(Mmonit_T *);
static void _gc_url(URL_T *);
static void _gc_request(Request_T *);


/**
 *  Release allocated memory.
 *
 *  @file
 */


/* ------------------------------------------------------------------ Public */


void gc() {
        Engine_destroyHostsAllow();
        if (Run.flags & Run_ProcessEngineEnabled) {
                delprocesstree(&oldptree, &oldptreesize);
                delprocesstree(&ptree, &ptreesize);
        }
        if (servicelist)
                _gc_service_list(&servicelist);
        if (servicegrouplist)
                _gc_servicegroup(&servicegrouplist);
        if (Run.httpd.credentials)
                _gcath(&Run.httpd.credentials);
        if (Run.maillist)
                gc_mail_list(&Run.maillist);
        if (Run.mailservers)
                _gc_mail_server(&Run.mailservers);
        if (Run.mmonits)
                _gc_mmonit(&Run.mmonits);
        if (Run.eventlist)
                gc_event(&Run.eventlist);
        FREE(Run.eventlist_dir);
        FREE(Run.mygroup);
        if (Run.httpd.flags & Httpd_Net) {
                FREE(Run.httpd.socket.net.address);
                FREE(Run.httpd.socket.net.ssl.pem);
                FREE(Run.httpd.socket.net.ssl.clientpem);
        } else if (Run.httpd.flags & Httpd_Unix) {
                FREE(Run.httpd.socket.unix.path);
        }
        FREE(Run.MailFormat.from);
        FREE(Run.MailFormat.subject);
        FREE(Run.MailFormat.message);
        FREE(Run.mail_hostname);
}


void gc_mail_list(Mail_T *m) {
        ASSERT(m);
        if ((*m)->next)
                gc_mail_list(&(*m)->next);
        FREE((*m)->to);
        FREE((*m)->from);
        FREE((*m)->replyto);
        FREE((*m)->subject);
        FREE((*m)->message);
        FREE(*m);
}


void gccmd(command_t *c) {
        ASSERT(c&&*c);
        for (int i = 0; (*c)->arg[i]; i++)
                FREE((*c)->arg[i]);
        FREE(*c);
}


void gc_event(Event_T *e) {
        ASSERT(e&&*e);
        if ((*e)->next)
                gc_event(&(*e)->next);
        (*e)->action = NULL;
        FREE((*e)->source);
        FREE((*e)->message);
        FREE(*e);
}


/* ----------------------------------------------------------------- Private */


static void _gc_service_list(Service_T *s) {
        ASSERT(s&&*s);
        if ((*s)->next)
                _gc_service_list(&(*s)->next);
        _gc_service(&(*s));
}


static void _gc_service(Service_T *s) {
        ASSERT(s&&*s);
        if ((*s)->program) {
                if ((*s)->program->P)
                        Process_free(&(*s)->program->P);
                if ((*s)->program->C)
                        Command_free(&(*s)->program->C);
                if ((*s)->program->args)
                        gccmd(&(*s)->program->args);
                StringBuffer_free(&((*s)->program->output));
                FREE((*s)->program);
        }
        if ((*s)->portlist)
                _gcportlist(&(*s)->portlist);
        if ((*s)->socketlist)
                _gcportlist(&(*s)->socketlist);
        if ((*s)->filesystemlist)
                _gcfilesystem(&(*s)->filesystemlist);
        if ((*s)->icmplist)
                _gcicmp(&(*s)->icmplist);
        if ((*s)->maillist)
                gc_mail_list(&(*s)->maillist);
        if ((*s)->resourcelist)
                _gcpql(&(*s)->resourcelist);
        if ((*s)->timestamplist)
                _gcptl(&(*s)->timestamplist);
        if ((*s)->actionratelist)
                _gcparl(&(*s)->actionratelist);
        if ((*s)->sizelist)
                _gcso(&(*s)->sizelist);
        if ((*s)->linkstatuslist)
                _gclinkstatus(&(*s)->linkstatuslist);
        if ((*s)->linkspeedlist)
                _gclinkspeed(&(*s)->linkspeedlist);
        if ((*s)->linksaturationlist)
                _gclinksaturation(&(*s)->linksaturationlist);
        if ((*s)->uploadbyteslist)
                _gcbandwidth(&(*s)->uploadbyteslist);
        if ((*s)->uploadpacketslist)
                _gcbandwidth(&(*s)->uploadpacketslist);
        if ((*s)->downloadbyteslist)
                _gcbandwidth(&(*s)->downloadbyteslist);
        if ((*s)->downloadpacketslist)
                _gcbandwidth(&(*s)->downloadpacketslist);
        if ((*s)->matchlist)
                _gcmatch(&(*s)->matchlist);
        if ((*s)->matchignorelist)
                _gcmatch(&(*s)->matchignorelist);
        if ((*s)->checksum)
                _gcchecksum(&(*s)->checksum);
        if ((*s)->perm)
                _gcperm(&(*s)->perm);
        if ((*s)->statuslist)
                _gcstatus(&(*s)->statuslist);
        if ((*s)->every.type == Every_Cron || (*s)->every.type == Every_NotInCron)
                FREE((*s)->every.spec.cron);
        if ((*s)->uid)
                _gcuid(&(*s)->uid);
        if ((*s)->euid)
                _gcuid(&(*s)->euid);
        if ((*s)->gid)
                _gcgid(&(*s)->gid);
        if ((*s)->pidlist)
                _gcpid(&(*s)->pidlist);
        if ((*s)->ppidlist)
                _gcppid(&(*s)->ppidlist);
        if ((*s)->fsflaglist)
                _gcfsflag(&(*s)->fsflaglist);
        if ((*s)->nonexistlist)
                _gcnonexist(&(*s)->nonexistlist);
        if ((*s)->dependantlist)
                _gcpdl(&(*s)->dependantlist);
        if ((*s)->start)
                gccmd(&(*s)->start);
        if ((*s)->stop)
                gccmd(&(*s)->stop);
        if ((*s)->action_DATA)
                _gc_eventaction(&(*s)->action_DATA);
        if ((*s)->action_EXEC)
                _gc_eventaction(&(*s)->action_EXEC);
        if ((*s)->action_INVALID)
                _gc_eventaction(&(*s)->action_INVALID);
        if ((*s)->action_MONIT_START)
                _gc_eventaction(&(*s)->action_MONIT_START);
        if ((*s)->action_MONIT_STOP)
                _gc_eventaction(&(*s)->action_MONIT_STOP);
        if ((*s)->action_MONIT_RELOAD)
                _gc_eventaction(&(*s)->action_MONIT_RELOAD);
        if ((*s)->action_ACTION)
                _gc_eventaction(&(*s)->action_ACTION);
        if ((*s)->eventlist)
                gc_event(&(*s)->eventlist);
        if ((*s)->inf) {
                if ((*s)->type == Service_Net)
                        Link_free(&((*s)->inf->priv.net.stats));
                FREE((*s)->inf);
        }
        FREE((*s)->name);
        FREE((*s)->path);
        (*s)->next = NULL;
        FREE(*s);
}


static void _gc_servicegroup(ServiceGroup_T *sg) {
        ASSERT(sg && *sg);
        if ((*sg)->next)
                _gc_servicegroup(&(*sg)->next);
        List_free(&(*sg)->members);
        FREE((*sg)->name);
        FREE(*sg);
}


static void _gc_request(Request_T *r) {
        ASSERT(r);
        if ((*r)->url)
                _gc_url(&(*r)->url);
#ifdef HAVE_REGEX_H
        if ((*r)->regex)
                regfree((*r)->regex);
#endif
        FREE((*r)->regex);
        FREE(*r);
}


static void _gc_url(URL_T *url) {
        ASSERT(url);
        FREE((*url)->url);
        FREE((*url)->protocol);
        FREE((*url)->user);
        FREE((*url)->password);
        FREE((*url)->hostname);
        FREE((*url)->path);
        FREE((*url)->query);
        FREE(*url);
}


static void _gc_mail_server(MailServer_T *s) {
        if (! s || ! *s)
                return;
        if ((*s)->next)
                _gc_mail_server(&(*s)->next);
        FREE((*s)->host);
        FREE((*s)->username);
        FREE((*s)->password);
        FREE((*s)->ssl.certmd5);
        FREE((*s)->ssl.clientpemfile);
        FREE(*s);
}


static void _gc_action(Action_T *a) {
        ASSERT(a&&*a);
        if ((*a)->exec)
                gccmd(&(*a)->exec);
        FREE(*a);
}


static void _gc_eventaction(EventAction_T *e) {
        ASSERT(e&&*e);
        _gc_action(&(*e)->failed);
        _gc_action(&(*e)->succeeded);
        FREE(*e);
}


static void _gcportlist(Port_T *p) {
        ASSERT(p&&*p);
        if ((*p)->next)
                _gcportlist(&(*p)->next);
        if ((*p)->action)
                _gc_eventaction(&(*p)->action);
        if ((*p)->url_request)
                _gc_request(&(*p)->url_request);
        if ((*p)->family == Socket_Unix) {
                FREE((*p)->target.unix.pathname);
        } else {
                FREE((*p)->target.net.SSL.certmd5);
                FREE((*p)->target.net.SSL.clientpemfile);
        }
        FREE((*p)->hostname);
        if ((*p)->protocol->check == check_http) {
                FREE((*p)->parameters.http.request);
                FREE((*p)->parameters.http.checksum);
                if ((*p)->parameters.http.headers) {
                        List_T l = (*p)->parameters.http.headers;
                        while (List_length(l) > 0) {
                                char *s = List_pop(l);
                                FREE(s);
                        }
                }
        } else if ((*p)->protocol->check == check_generic) {
                if ((*p)->parameters.generic.sendexpect)
                        _gcgeneric(&(*p)->parameters.generic.sendexpect);
        } else if ((*p)->protocol->check == check_mysql) {
                FREE((*p)->parameters.mysql.username);
                FREE((*p)->parameters.mysql.password);
        } else if ((*p)->protocol->check == check_sip) {
                FREE((*p)->parameters.sip.target);
        } else if ((*p)->protocol->check == check_radius) {
                FREE((*p)->parameters.radius.secret);
        } else if ((*p)->protocol->check == check_websocket) {
                FREE((*p)->parameters.websocket.host);
                FREE((*p)->parameters.websocket.origin);
                FREE((*p)->parameters.websocket.request);
        }
        FREE(*p);
}


static void _gcfilesystem(Filesystem_T *d) {
        ASSERT(d&&*d);
        if ((*d)->next)
                _gcfilesystem(&(*d)->next);
        if ((*d)->action)
                _gc_eventaction(&(*d)->action);
        FREE(*d);
}


static void _gcicmp(Icmp_T *i) {
        ASSERT(i&&*i);
        if ((*i)->next)
                _gcicmp(&(*i)->next);
        if ((*i)->action)
                _gc_eventaction(&(*i)->action);
        FREE(*i);
}


static void _gcpql(Resource_T *q) {
        ASSERT(q);
        if ((*q)->next)
                _gcpql(&(*q)->next);
        if ((*q)->action)
                _gc_eventaction(&(*q)->action);
        FREE(*q);
}


static void _gcptl(Timestamp_T *p) {
        ASSERT(p);
        if ((*p)->next)
                _gcptl(&(*p)->next);
        if ((*p)->action)
                _gc_eventaction(&(*p)->action);
        FREE(*p);
}


static void _gcparl(ActionRate_T *ar) {
        ASSERT(ar);
        if ((*ar)->next)
                _gcparl(&(*ar)->next);
        if ((*ar)->action)
                _gc_eventaction(&(*ar)->action);
        FREE(*ar);
}


static void _gcso(Size_T *s) {
        ASSERT(s);
        if ((*s)->next)
                _gcso(&(*s)->next);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE(*s);
}

static void _gclinkstatus(LinkStatus_T *l) {
        ASSERT(l);
        if ((*l)->next)
                _gclinkstatus(&(*l)->next);
        if ((*l)->action)
                _gc_eventaction(&(*l)->action);
        FREE(*l);
}


static void _gclinkspeed(LinkSpeed_T *l) {
        ASSERT(l);
        if ((*l)->next)
                _gclinkspeed(&(*l)->next);
        if ((*l)->action)
                _gc_eventaction(&(*l)->action);
        FREE(*l);
}


static void _gclinksaturation(LinkSaturation_T *l) {
        ASSERT(l);
        if ((*l)->next)
                _gclinksaturation(&(*l)->next);
        if ((*l)->action)
                _gc_eventaction(&(*l)->action);
        FREE(*l);
}


static void _gcbandwidth(Bandwidth_T *b) {
        ASSERT(b);
        if ((*b)->next)
                _gcbandwidth(&(*b)->next);
        if ((*b)->action)
                _gc_eventaction(&(*b)->action);
        FREE(*b);
}

static void _gcmatch(Match_T *s) {
        ASSERT(s);
        if ((*s)->next)
                _gcmatch(&(*s)->next);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE((*s)->match_path);
        FREE((*s)->match_string);
#ifdef HAVE_REGEX_H
        if ((*s)->regex_comp) {
                regfree((*s)->regex_comp);
                FREE((*s)->regex_comp);
        }
#endif
        FREE(*s);
}


static void _gcchecksum(Checksum_T *s) {
        ASSERT(s);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE(*s);
}


static void _gcperm(Perm_T *s) {
        ASSERT(s);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE(*s);
}


static void _gcstatus(Status_T *s) {
        ASSERT(s);
        if ((*s)->next)
                _gcstatus(&(*s)->next);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE(*s);
}


static void _gcuid(Uid_T *s) {
        ASSERT(s);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE(*s);
}


static void _gcgid(Gid_T *s) {
        ASSERT(s);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE(*s);
}


static void _gcpid(Pid_T *s) {
        ASSERT(s);
        if ((*s)->next)
                _gcpid(&(*s)->next);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE(*s);
}


static void _gcppid(Pid_T *s) {
        ASSERT(s);
        if ((*s)->next)
                _gcppid(&(*s)->next);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE(*s);
}


static void _gcfsflag(Fsflag_T *s) {
        ASSERT(s);
        if ((*s)->next)
                _gcfsflag(&(*s)->next);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE(*s);
}


static void _gcnonexist(Nonexist_T *s) {
        ASSERT(s);
        if ((*s)->next)
                _gcnonexist(&(*s)->next);
        if ((*s)->action)
                _gc_eventaction(&(*s)->action);
        FREE(*s);
}


static void _gcpdl(Dependant_T *d) {
        ASSERT(d);
        if ((*d)->next)
                _gcpdl(&(*d)->next);
        FREE((*d)->dependant);
        FREE(*d);
}


static void _gcgeneric(Generic_T *g) {
        ASSERT(g);
        if ((*g)->next)
                _gcgeneric(&(*g)->next);
        FREE((*g)->send);
#ifdef HAVE_REGEX_H
        if ((*g)->expect != NULL)
                regfree((*g)->expect);
#endif
        FREE((*g)->expect);
        FREE(*g);

}


static void _gcath(Auth_T *c) {
        ASSERT(c);
        if ((*c)->next)
                _gcath(&(*c)->next);
        FREE((*c)->uname);
        FREE((*c)->passwd);
        FREE((*c)->groupname);
        FREE(*c);
}


static void _gc_mmonit(Mmonit_T *recv) {
        ASSERT(recv);
        if ((*recv)->next)
                _gc_mmonit(&(*recv)->next);
        _gc_url(&(*recv)->url);
        FREE((*recv)->ssl.certmd5);
        FREE((*recv)->ssl.clientpemfile);
        FREE(*recv);
}

