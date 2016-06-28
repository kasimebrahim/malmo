// --------------------------------------------------------------------------------------------------
//  Copyright (c) 2016 Microsoft Corporation
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
//  associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// --------------------------------------------------------------------------------------------------

// Malmo:
#include <AgentHost.h>
#include <ClientPool.h>
using namespace malmo;

// STL:
#include <cstdlib>
#include <exception>
#include <iostream>
using namespace std;

int main(int argc, const char **argv)
{
    AgentHost agent_host;

    try
    {
        agent_host.parseArgs(argc, argv);
    }
    catch( const exception& e )
    {
        cout << "ERROR: " << e.what() << endl;
        cout << agent_host.getUsage() << endl;
        return EXIT_SUCCESS;
    }
    if( agent_host.receivedArgument("help") )
    {
        cout << agent_host.getUsage() << endl;
        return EXIT_SUCCESS;
    }

    MissionSpec my_mission;
    my_mission.timeLimitInSeconds(10);
    my_mission.requestVideo( 320, 240 );
    my_mission.rewardForReachingPosition(19.5f,0.0f,19.5f,100.0f,1.1f);

    MissionRecordSpec my_mission_record("./saved_data.tgz");
    my_mission_record.recordCommands();
    my_mission_record.recordMP4(20, 400000);
    my_mission_record.recordRewards();
    my_mission_record.recordObservations();

    try {
        agent_host.startMission( my_mission, my_mission_record );
    }
    catch (exception& e) {
        cout << "Error starting mission: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    boost::shared_ptr<const WorldState> world_state;

    cout << "Waiting for the mission to start" << flush;
    do {
        cout << "." << flush;
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        world_state = agent_host.getWorldState();
        for( TimestampedString error : world_state->errors )
            cout << "Error: " << error.text << endl;
    } while (!world_state->is_mission_running);
    cout << endl;

    // main loop:
    do {
        agent_host.sendCommand("move 1");
        {
            ostringstream oss;
            oss << "turn " << rand() / float(RAND_MAX);
            agent_host.sendCommand(oss.str());
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(500));
        world_state = agent_host.getWorldState();
        cout << "video,observations,rewards received: "
             << world_state->number_of_video_frames_since_last_state << ","
             << world_state->number_of_observations_since_last_state << ","
             << world_state->number_of_rewards_since_last_state << endl;
        for( TimestampedFloat reward : world_state->rewards )
            cout << "Summed reward: " << reward.value << endl;
        for( TimestampedString error : world_state->errors )
            cout << "Error: " << error.text << endl;
    } while (world_state->is_mission_running);

    cout << "Mission has stopped." << endl;

    return EXIT_SUCCESS;
}
