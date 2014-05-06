/*******************************************************************************

    Copyright 2014 Ben Wojtowicz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************

    File: LTE_fdd_enb_timer.cc

    Description: Contains all the implementations for the LTE FDD eNodeB
                 timer class.

    Revision History
    ----------    -------------    --------------------------------------------
    05/04/2014    Ben Wojtowicz    Created file

*******************************************************************************/

/*******************************************************************************
                              INCLUDES
*******************************************************************************/

#include "LTE_fdd_enb_timer.h"

/*******************************************************************************
                              DEFINES
*******************************************************************************/


/*******************************************************************************
                              TYPEDEFS
*******************************************************************************/


/*******************************************************************************
                              GLOBAL VARIABLES
*******************************************************************************/


/*******************************************************************************
                              CLASS IMPLEMENTATIONS
*******************************************************************************/

/******************/
/*    Callback    */
/******************/
timer_cb::timer_cb()
{
}
timer_cb::timer_cb(FuncType f, void* o)
{
    func = f;
    obj  = o;
}
void timer_cb::operator()(uint32 id)
{
    return (*func)(obj, id);
}

/********************************/
/*    Constructor/Destructor    */
/********************************/
LTE_fdd_enb_timer::LTE_fdd_enb_timer(uint32   seconds,
                                     uint32   _id,
                                     timer_cb _cb)
{
    cb              = _cb;
    id              = _id;
    expiry_seconds  = seconds;
    current_seconds = 0;
}
LTE_fdd_enb_timer::~LTE_fdd_enb_timer()
{
}

/****************************/
/*    External Interface    */
/****************************/
void LTE_fdd_enb_timer::increment(void)
{
    current_seconds++;

    if(expired())
    {
        cb(id);
    }
}
bool LTE_fdd_enb_timer::expired(void)
{
    bool exp = false;

    if(current_seconds > expiry_seconds)
    {
        exp = true;
    }

    return(exp);
}
