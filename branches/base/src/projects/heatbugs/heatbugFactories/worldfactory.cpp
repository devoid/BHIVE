#include "worldfactory.hh"

using namespace Heatbug;

WorldFactory::WorldFactory() { }

WorldFactory::~WorldFactory() { }

Agent* WorldFactory::createAgent() {
	// create the agent and get its datbase
	Agent *bond = new Agent();
	Database *db = bond->getDatabase();

	// create the data items that are needed for this agent and add
	// the data items to the agent's database

	// clock
	DoubleData *celltime = new DoubleData("worldtime", 0.0);
	db->addData(celltime);

	// time interval
	DoubleData *dt = new DoubleData("dt",0.01);
	db->addData(dt);

	// output interval
	DoubleData *oi = new DoubleData("output_interval", this->output_interval);
	db->addData(oi);

	DoubleData *noti = new DoubleData("next_output_time", this->output_interval);
	db->addData(noti);

	// diffusion coefficient
	DoubleData *diff = new DoubleData("diffusion_coeff", 0.1);
	db->addData(diff);

	// dissipation rate
	DoubleData *dis = new DoubleData("dissipation_rate", 0.00333333);
	db->addData(dis);

	// size of the grids in the system. all grids will be quadratic.
	IntegerData *gs = new IntegerData("grid_size", this->gridsize);
	db->addData(gs);

	// number_bugs
	IntegerData *bn = new IntegerData("number_bugs", number_bugs);
	db->addData(bn);

	// temperature grid.
	TVectorData<TVectorData<DoubleData>* > *temperature_grid = new TVectorData<TVectorData<DoubleData>* >("temperature_grid","tvectordata_double");
	temperature_grid->reserveSize(gridsize);
	for (int i=0; i<gridsize; i++) {
		temperature_grid->at(i) = new TVectorData<DoubleData>("grid_row", "tvectordata_double");
		temperature_grid->at(i)->reserveSize(gridsize);
		/// initialise the temperature to zero.
		for (int j=0; j<gridsize; j++)
			temperature_grid->at(i)->at(j).setDouble(0);
	}
	// add data to agent's database
	db->addData(temperature_grid);

	// bug grid: bug_grid[i][j] returns the id of the bug-agent that is lives on grid position i,j.
	// if position i/j is empty, bug_grid[i][j] returns zero
	TVectorData<TVectorData<IntegerData>* > *bug_grid = new TVectorData<TVectorData<IntegerData>* >("bug_grid", "tvectordata_int");
	bug_grid->reserveSize(gridsize);
	for (int i=0; i<gridsize; i++) {
		// the name of this data item will not be used
		bug_grid->at(i) = new TVectorData<IntegerData>("grid_row","tvectordata_int");
		bug_grid->at(i)->reserveSize(gridsize);
		/// initialise the temperature to zero.
		for (int j=0; j<gridsize; j++)
			bug_grid->at(i)->at(j).setInteger(0);
	}
	// add data to agent's database
	db->addData(bug_grid);

	// bug-positions. vector storing the x and y index of where bug i lives.
	TVectorData<TVectorData<IntegerData>* > *bug_positions = new TVectorData<TVectorData<IntegerData>* >("bug_positions", "tvectordata_int");
	bug_positions->reserveSize(number_bugs);
	// reserve size
	for (int i=0; i<number_bugs; i++) {
		bug_positions->at(i) = new TVectorData<IntegerData> ("bug_position", "tvectordata_int");
		bug_positions->at(i)->reserveSize(2);
	}
	// add data to agent's database
	db->addData(bug_positions);

	// temporary matrix storing the desired bug positions.
	TVectorData<TVectorData<TVectorData<IntegerData>* >* > *desired_bug_grid = new TVectorData<TVectorData<TVectorData<IntegerData>* >* > ("desired_bug_grid", "no_type_given");

	desired_bug_grid->reserveSize(gridsize);
	for (int i=0; i<gridsize; i++) {
		desired_bug_grid->at(i) = new TVectorData<TVectorData<IntegerData>* >("grid_row", "no_type_given");
		desired_bug_grid->at(i)->reserveSize(gridsize);
		for (int j=0; j<gridsize; j++)
			desired_bug_grid->at(i)->at(j) = new TVectorData<IntegerData> ("no_name_given", "no_type_given");
	}
	db->addData(desired_bug_grid);

	// vector for temporary storage
	TVectorData<TVectorData<IntegerData>* > *desired_bug_positions = new TVectorData<TVectorData<IntegerData>* >("desired_bug_positions", "tvectordata_int");
	desired_bug_positions->reserveSize(number_bugs);
	for (int i=0; i<number_bugs; i++) {
		desired_bug_positions->at(i) = new TVectorData<IntegerData> ("bug_position", "tvectordata_int");
		desired_bug_positions->at(i)->reserveSize(2);
	}
	db->addData(desired_bug_positions);

	// we will randomly initialise the positions of the bugs here. in general, it would be a lot nicer to have
	// an initialiser class with which one could choose between different initialisation methods. however, since this
	// is only an example on how to do agent/rule-based simulations in the hive, i do not want to spend too much time
	// implementing this. hence i do it this way!
	int x,y;
	for (int i=0; i<number_bugs; i++) {
		// randomly select a position
		do {
			x = Util::RANDOM_INT(0,gridsize-1);
			y = Util::RANDOM_INT(0,gridsize-1);
		} while (bug_grid->at(x)->at(y).getInteger() != 0);
		// update bug_grid
		bug_grid->at(x)->at(y).setInteger(i+1);
		// update bug_positions
		bug_positions->at(i)->at(0).setInteger(x);
		bug_positions->at(i)->at(1).setInteger(y);
	}


	// finally add actions, simulators and message generators to the world agent
	this->addActionsToAgentsActionSet(bond);
	this->addSimulatorToAgent(bond);
	this->addMessageGeneratorsToAgent(bond);

	return bond;
}

// for the moment, this method does not do anything but throwing an exception
Agent* WorldFactory::createAgent(Action **as) {
	throw HiveException("in Heatbug namespace: method not implmented", "Agent* WorldFactory::createAgent(Action **as)");
}

// for the moment, this method does not do anything but throwing an exception
Agent* WorldFactory::duplicateAgent(Agent *ag) {
	throw HiveException("in Heatbug namespace: method not implmented", "Agent* WorldFactory::duplicateAgent(Agent *a)");
}

void WorldFactory::finalise() { }

void WorldFactory::addActionsToAgentsActionSet(Agent *ag) {
	WorldUpdateDesiredMovementOfBugAction *wudmoba = new WorldUpdateDesiredMovementOfBugAction();
	wudmoba->setAgent(ag);
	ag->addAction(wudmoba);

	WorldUpdateTemperatureProducedByBugAction *wutpbba = new WorldUpdateTemperatureProducedByBugAction();
	wutpbba->setAgent(ag);
	ag->addAction(wutpbba);
}

void WorldFactory::addSimulatorToAgent(Agent *ag) {
	DiffusionSimulator *ds = new DiffusionSimulator();
	ds->setAgent(ag);
	ag->addSimulator(ds);

	SimulatorForResolvingConflicts *sfrc = new SimulatorForResolvingConflicts();
	sfrc->setAgent(ag);
	ag->addSimulator(sfrc);

	TimeKeeper *tk = new TimeKeeper(ag,"worldtime");
	ag->addSimulator(tk);
}

void WorldFactory::addMessageGeneratorsToAgent(Agent *ag) {
	WorldNotifyBugAboutLocalTemperatureMessageGenerator *wnbaltmg = new WorldNotifyBugAboutLocalTemperatureMessageGenerator(ag);
	ag->addMessageGenerator(wnbaltmg);

	WorldTellSpecialAgentAboutBugPositionsMessageGenerator *wtsaabpmg = new WorldTellSpecialAgentAboutBugPositionsMessageGenerator(ag);
	ag->addMessageGenerator(wtsaabpmg);

	WorldTellSpecialAgentAboutTemperatureGridMessageGenerator *wtsaatgmg = new WorldTellSpecialAgentAboutTemperatureGridMessageGenerator(ag);
	ag->addMessageGenerator(wtsaatgmg);
}
