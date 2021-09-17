#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <regex>
#include <math.h>
#include <set>
#include <cstdio>

using namespace std;


int calltime_to_sec(string calltime);

struct participants {
	string name;
	int ucastnik = 0;
	int v_hovoru_sec = 0;
	string v_hovoru = "default";
	int pocet_odpojeni = -1;
	string pocet_odpojeni_str = " ";
	string sam_v_hovoru = "sam(a)";

	participants(string name) {
		this->name = name;
	}

	void v_hovoru_to_string() {
		if (v_hovoru_sec == 0)
			v_hovoru = "        ";
		else {
			int div = v_hovoru_sec;
			v_hovoru = "";
			for (int i = 2; i >= 0; i--) {
				string p = to_string(div / (int)pow(60, i));
				if (p.size() == 1)
					v_hovoru.append("0");
				v_hovoru.append(p);
				v_hovoru.append(":");
				div = div % (int)pow(60, i);
			}
			v_hovoru.erase(v_hovoru.size() - 1);
		}
	}

	void pocet_odpojeni_to_string() {
		if (pocet_odpojeni == -1)
			pocet_odpojeni_str = " ";
		else
			pocet_odpojeni_str = to_string(pocet_odpojeni);
	}

	void print() {
		cout << name << '\t' << "ucastnik: " << ucastnik << '\t' << "v hovoru: " << v_hovoru << '\t';
		cout << "pocet odpojeni: " << pocet_odpojeni << '\t' << "sam v hovoru: " << sam_v_hovoru << endl;
	}

};

struct ind_calltime {
	string name;
	string calldate_start;
	string jointime;
	string joindate;
	string leavetime;
	string leavedate;
	int row = 0;
	int solo_calltime = 0;
	int next_time = 0;			// nejblizsi dalsi cas zmeny
	int status = 3;		// 1 - jointime, 2 - leavetime, 3 - novy jointime

	void set_next_time() {
		if (status == 1) {
			next_time = calltime_to_sec(jointime);
			if (calldate_start.compare(joindate) != 0)
				next_time += 24 * 3600;
			status++;
		}
		else if (status == 2) {
			next_time = calltime_to_sec(leavetime);
			if (calldate_start.compare(leavedate) != 0)
				next_time += 24 * 3600;
			status++;
		}
		else if (status == 3)
			status++;
	}
};

struct call {
	string datum_hovoru;
	int ucastnici[3] = {};
	string delka_hovoru;
	string v_hovoru[4];
	string pocet_odpojeni[3];
	string sam_v_hovoru[3];
	int organizer = 0;

	call() {}

	call(string datum_hovoru, int ucastnici[3], string delka_hovoru, string v_hovoru[4], 
		string pocet_odpojeni[3], string sam_v_hovoru[3], string org_name) {
		this->datum_hovoru = datum_hovoru;
		set_ucastnici(ucastnici);
		this->delka_hovoru = delka_hovoru;
		set_v_hovoru(v_hovoru);
		set_pocet_odpojeni(pocet_odpojeni);
		set_sam_v_hovoru(sam_v_hovoru);
		set_organizer(org_name);
	}

	void set_ucastnici(int ucastnici[3]) {
		for (int i = 0; i < 3; i++)
			this->ucastnici[i] = ucastnici[i];
	}

	void set_v_hovoru(string v_hovoru[4]) {
		for (int i = 0; i < 4; i++)
			this->v_hovoru[i] = v_hovoru[i];
	}

	void set_pocet_odpojeni(string pocet_odpojeni[3]) {
		for (int i = 0; i < 3; i++)
			this->pocet_odpojeni[i] = pocet_odpojeni[i];
	}

	void set_sam_v_hovoru(string sam_v_hovoru[3]) {
		for (int i = 0; i < 3; i++)
			this->sam_v_hovoru[i] = sam_v_hovoru[i];
	}

	void set_organizer(string name) {
		if (name.compare("Kernel") == 0)
			this->organizer = 1;
		else if (name.compare("Vacule") == 0)
			this->organizer = 2;
		else
			this->organizer = 3;
	}

	void print() {
		string space = "  ";
		cout << datum_hovoru << space;
		for (int i : ucastnici)
			cout << i << space;
		cout << delka_hovoru << space;
		for (string s : v_hovoru)
			cout << s << space;
		for (string s : pocet_odpojeni)
			cout << s << space;
		for (string s : sam_v_hovoru)
			cout << s << space;
		cout << organizer << endl;
	}

	void date_to_int(string datum_hovoru, int array[]) {
		regex reg("(\\d+)(?:\\D)(\\d+)(?:\\D)(\\d+)");
		smatch sm;
		regex_match(datum_hovoru, sm, reg);
		for (int i = 0; i < 3; i++)
			array[i] = stoi(sm[i + 1].str());
	}

	bool operator < (call h) {
		int first[3];
		int second[3];
		date_to_int(this->datum_hovoru, first);
		h.date_to_int(h.datum_hovoru, second);

		for (int i = 2; i >= 0; i--) {
			if (first[i] < second[i])
				return true;
			if (first[i] > second[i])
				return false;
		}
		return true;
	}
};

struct mensi {
	bool operator () (call h, call k) const {
		int first[3];
		int second[3];
		h.date_to_int(h.datum_hovoru, first);
		k.date_to_int(k.datum_hovoru, second);

		for (int i = 2; i >= 0; i--) {
			if (first[i] < second[i])
				return true;
			if (first[i] > second[i])
				return false;
		}
		return false;
	}
};

string call_date(string end_time, string format_by) {

	string result = "";
	regex reg("(?:\\D+)(\\d+)(?:\\D+)(\\d+)(?:\\D+)(\\d+)(?:\.+)");
	smatch sm;

	regex_match(end_time, sm, reg);

	if (sm.empty()) {
		cout << "Regex  in function 'call_date()' is empty." << endl;
		return "stop";
	}

	if (format_by.compare("Kernel") == 0) {
		for (int i = 2; i > 0; i--) {
			if (sm[i].str().size() == 1)
				result.append("0");
			result.append(sm[i].str());
			result.append(".");
		}
	}
	else {
		for (int i = 1; i < sm.size() - 1; i++) {
			result.append(sm[i].str());
			result.append(".");
		}
	}
	result.append(sm[3].str());

	return result;
}

int num_of_partic(string participants) {

	regex reg("(?:\\D+)(\\d+)(?:\.*)");
	smatch sm;

	regex_match(participants, sm, reg);
	if (sm.empty()) {
		cout << "Regex  in function 'num_of_partic()' is empty." << endl;
		return 0;
	}
	else
		return stoi(sm[1].str());
}

string to_24h_format(string time, string part) {

	regex reg("(\\d+)(\.+)");
	smatch sm;

	regex_match(time, sm, reg);

	if (sm.empty())
		cout << "Regex  in function 'to_24h_format()' is empty." << endl;

	int t = stoi(sm[1].str());

	if (part.compare("AM") == 0) {
		if (t == 12)
			t = 0;
	}
	else {
		if (t != 12)
			t += 12;
	}

	return to_string(t) + sm[2].str();
}

string call_duration(string start_date, string start_time, string end_date, string end_time) {

	regex reg("(\\d+)(?:\\D+)(\\d+)(?:\\D+)(\\d+)");
	smatch sm_start;
	smatch sm_end;

	regex_match(start_time, sm_start, reg);
	regex_match(end_time, sm_end, reg);

	int start = 0;
	int end = 0;

	for (int i = 0; i < sm_start.size() - 1; i++) {
		start += stoi(sm_start[sm_start.size() - 1 - i].str()) * (int)pow(60, i);
	}
	for (int i = 0; i < sm_end.size() - 1; i++) {
		end += stoi(sm_end[sm_end.size() - 1 - i].str()) * (int)pow(60, i);
	}
	if (start_date.compare(end_date) != 0)
		end += 24 * 3600;

	int div = end - start;
	string result = "";
	for (int i = 2; i >= 0; i--) {
		string p = to_string(div / (int)pow(60, i));
		if (p.size() == 1)
			result.append("0");
		result.append(p);
		result.append(":");
		div = div % (int)pow(60, i);
	}
	return result.erase(result.size()-1);
}

string calltime(string start, string end, string format_by) {

	regex reg("(?:\\D+)(\\d+\\D+\\d+\\D+\\d+)(?:\\D+)(\\d+\\D+\\d+\\D+\\d+)(?:[^[:alpha:]]*)([[:alpha:]]*)");
	smatch sm_start;
	smatch sm_end;

	regex_match(start, sm_start, reg);
	regex_match(end, sm_end, reg);

	if (sm_start.empty() || sm_end.empty()) {
		cout << "Regex  in function 'calltime()' is empty." << endl;
		return "stop";
	}

	string data[4];

	data[0] = sm_start[1].str();
	data[2] = sm_end[1].str();

	if (format_by.compare("Kernel") == 0) {
		data[1] = to_24h_format(sm_start[2].str(), sm_start[3].str());
		data[3] = to_24h_format(sm_end[2].str(), sm_end[3].str());
	}
	else {
		data[1] = sm_start[2].str();
		data[3] = sm_end[2].str();
	}

	return call_duration(data[0], data[1], data[2], data[3]);
}

string from_who(string line) {
	string organizer = line.erase(line.find('\t', 1));

	if (organizer.compare("Jinda Jiri") == 0)
		return "Kernel";
	else if (organizer.compare("Cukr Matej") == 0)
		return "Krystal";
	else
		return "Vacule";
}

string make_new_file_name(string datum_hovoru, string delka_hovoru) {

	string result = "";

	regex reg("(?:\\D*)(\\d+)(?:\\D+)(\\d+)(?:\\D+)(\\d+)(?:\.*)");
	smatch sm;

	regex_match(datum_hovoru, sm, reg);

	for (int i = 3; i > 1; i--) {
		result.append(sm[i].str());
		result.append("-");
	}
	result.append(sm[1].str());
	result.append("_");

	regex_match(delka_hovoru, sm, reg);

	for (int i = 1; i < 4; i++) {
		result.append(sm[i].str());
	}
	result.append(".csv");

	return result;
}

string sec_to_calltime(int sec) {

	string result = "";

	for (int i = 2; i >= 0; i--) {
		int n = sec / (int)pow(60, i);
		string t = to_string(n);
		if (t.size() == 1)
			result.append("0");
		result.append(t);
		result.append(":");
		sec = sec - (n * (int)pow(60, i));
	}
	result.resize(result.size() - 1);

	return result;
}

int calltime_to_sec(string calltime) {
	int result = 0;
	int exp = 2;
	regex reg("(\\d+)(?:\\D+)(\\d+)(?:\\D+)(\\d+)");
	smatch sm;
	regex_match(calltime, sm, reg);

	for (int i = 1; i < 4; i++) {
		result += stoi(sm[i].str()) * (int)pow(60, exp);
		exp--;
	}
	return result;
}

int ind_call_duration_to_sec(string time) {
	regex reg("(?:\\W*)(\\w+)");
	smatch sm;
	int seconds = 0;

	while (true) {
		regex_search(time, sm, reg);
		if (sm.empty())
			break;
		string part = sm[1].str();
		char interval = part[part.size()-1];
		part.pop_back();
		int number = stoi(part);
		if (interval == 'h')
			number *= 3600;
		else if (interval == 'm')
			number *= 60;
		seconds += number;
		time = sm.suffix();
	}
	return seconds;
}

void set_joins_and_leaves(ind_calltime &person, string line, string format_by) {
	
	string join = line;
	join.erase(0, join.find('\t') + 1);
	join.erase(join.find('\t'));
	string leave = line;
	leave.erase(0, leave.find('\t', leave.find('\t') + 1) + 1);
	leave.erase(leave.find('\t'));

	regex reg("(\\d+\\D+\\d+\\D+\\d+)(?:\\D+)(\.+)");
	smatch sm;

	regex_match(join, sm, reg);
	person.joindate = sm[1].str();
	person.jointime = sm[2].str();
	regex_match(leave, sm, reg);
	person.leavedate = sm[1].str();
	person.leavetime = sm[2].str();

	if (format_by.compare("Kernel") == 0) {
		reg.assign("(\\d+\\D+\\d+\\D+\\d+)(?:[^[:alpha:]]*)([[:alpha:]]*)");
		regex_match(person.jointime, sm, reg);
		person.jointime = to_24h_format(sm[1].str(), sm[2].str());
		regex_match(person.leavetime, sm, reg);
		person.leavetime = to_24h_format(sm[1].str(), sm[2].str());
	}
}

void load_time(ind_calltime &person, vector<string> data, string format_by) {

	bool found = false;

	for (int i = person.row; i < data.size(); i++) {
		string line = data[i];
		line.erase(line.find('\t'));
		if (line.compare(person.name) == 0) {
			set_joins_and_leaves(ref(person), data[i], format_by);
			person.status = 1;
			person.row = i + 1;
			found = true;
			break;
		}
	}

	if (!found)
		person.next_time = INT_MAX;
}

void alone_and_together_call_time(participants& kernel, participants& vacule, participants& krystal,
	vector<string> data, string &spolecne_v_hovoru, string orig_calldate, string format_by) {

	participants* parts[] = { &kernel, &vacule, &krystal };

		// specialni pripad - solo hovor
	if (data.size() == 1) {
		for (participants* p : parts) {
			if (p->ucastnik == 1)
				p->sam_v_hovoru = p->v_hovoru;
			else
				p->sam_v_hovoru = "        ";
		}
		spolecne_v_hovoru = "        ";
		return;
	}

	ind_calltime calltimes[3];

	for (int i = 0; i < 3; i++) {
		calltimes[i] = ind_calltime();				// poradi Kernel, Vacule, Krystal
		calltimes[i].name = parts[i]->name;
		calltimes[i].calldate_start = orig_calldate;
	}

	int in_call = 0;								// pocet lidi prave v hovoru
	int together_calltime = 0;						// v sekundach
	int start;
	bool who_in_call[] = { false, false, false };	// Kernel, Vacule, Krystal
	string names[] = { "Jinda Jiri", "Vaculkova Leona", "Cukr Matej" };
	ind_calltime* chosen_one = &ind_calltime();

	for (int i = 0; i < size(calltimes); i++) {
		load_time(ref(calltimes[i]), data, format_by);
		calltimes[i].set_next_time();
	}
	start = min({ calltimes[0].next_time, calltimes[1].next_time, calltimes[2].next_time });

	do {
		int end = min({ calltimes[0].next_time, calltimes[1].next_time, calltimes[2].next_time });

		if (in_call == 1) {
			for (int i = 0; i < 3; i++) {
				if (who_in_call[i])
					calltimes[i].solo_calltime += end - start;
			}
		}
		else if (in_call == 3)
			together_calltime += end - start;
		for (int i = 0; i < size(calltimes); i++) {
			if (calltimes[i].next_time == end) {
				chosen_one = &calltimes[i];
				break;
			}
		}
		start = end;
		for (int i = 0; i < size(names); i++) {
			if (names[i].compare(chosen_one->name) == 0) {
				if (chosen_one->status == 2) {
					who_in_call[i] = true;
					in_call++;
					break;
				}
				else {
					who_in_call[i] = false;
					in_call--;
					break;
				}
			}
		}
		chosen_one->set_next_time();
		if (chosen_one->status == 4) {
			load_time(*chosen_one, data, format_by);
			chosen_one->set_next_time();
		}
	} while (in_call > 0);

	if (together_calltime == 0)
		spolecne_v_hovoru = "        ";
	else
		spolecne_v_hovoru = sec_to_calltime(together_calltime);
	for (int i = 0; i < 3; i++) {
		if (calltimes[i].solo_calltime == 0)
			parts[i]->sam_v_hovoru = "        ";
		else
			parts[i]->sam_v_hovoru = sec_to_calltime(calltimes[i].solo_calltime);
	}
}

void individual_stats(participants &kernel, participants &vacule, participants &krystal, vector<string> data) {

	participants* parts[] = { &kernel, &vacule, &krystal };

	for (string line : data) {
		participants* chosen_one;
		string dur = line;
		string who = line;
		who.erase(line.find('\t', 1));
		for (participants* p : parts) {
			if (p->name.compare(who) == 0)
				chosen_one = p;
		}

		int pos = line.find('\t', line.find('\t', line.find('\t') + 1) + 1);
		dur.erase(0, pos + 1);
		dur.erase(dur.find('\t'));

		chosen_one->ucastnik = 1;
		chosen_one->pocet_odpojeni++;
		chosen_one->v_hovoru_sec += ind_call_duration_to_sec(dur);
	}
	
	for (participants* p : parts) {
		p->v_hovoru_to_string();
		p->pocet_odpojeni_to_string();
		p->v_hovoru_sec = 0;
	}
}

bool read_from_file(string in_file, vector<string> &lines) {

	fstream report;

	try {
		report.open(in_file, ios::in);
		if (!report)
			throw runtime_error("File not found ");
		if (!report.is_open())
			throw "Could not open file ";
		int i = 0;
		while (!report.eof()) {
			string line;
			if (i == 0 || i == 2 || i == 5 || i == 6)
				getline(report, line);
			else {
				char c;
				report.get(c);
				while (c != '\n') {
					if (c != '\0') {
						line += c;
						report.get(c);
					}
					else {
						report.get(c);
						if (c == '\0')
							break;
					}
				}
				lines.push_back(line);
			}
			i++;
		}
		report.close();
	}
	catch (exception const& s) {
		cout << s.what() << in_file << endl;
		return false;
	}
	catch (string s) {
		cout << s << in_file << endl;
		return false;
	}

	return true;
}

bool write_to_file(string out_file, multiset<call, mensi> all_calls) {

	fstream report;

	try {
		report.open("reports\\summary_report.csv", ios::out);
		if (!report.is_open())
			throw "Could not open file ";

		string comma = ",";
		for (call hovor : all_calls) {
			report << hovor.datum_hovoru << comma;
			for (int i : hovor.ucastnici)
				report << i << comma;
			report << hovor.delka_hovoru << comma;
			for (string s : hovor.v_hovoru)
				report << s << comma;
			for (string s : hovor.pocet_odpojeni)
				report << s << comma;
			for (string s : hovor.sam_v_hovoru)
				report << s << comma;
			report << hovor.organizer << '\n';
		}
		report.close();
	}
	catch (string s) {
		cout << s << "summary_report" << endl;
		return false;
	}

	return true;
}

bool process_file(string in_file, string out_file, multiset<call, mensi> &all_calls, string & new_file_name) {

	vector<string> lines;

	if (!read_from_file(in_file, ref(lines))) {
		cout << "Error occured while reading a file." << endl;
		return false;
	}

	if (lines.size() < 1) {
		cout << "No data were read from the input file." << endl;
		return false;
	}
	lines.pop_back();

	//for (auto line : lines)
	//	cout << line << endl;
	//cout << endl << endl;

	string format_by = from_who(lines[3]);

	int ucastnici_celkem = num_of_partic(lines[0]);
	if (ucastnici_celkem == 0)
		return false;
	string datum_hovoru = call_date(lines[2], format_by);
	if (datum_hovoru.compare("stop") == 0)
		return false;
	string delka_hovoru = calltime(lines[1], lines[2], format_by);
	if (delka_hovoru.compare("stop") == 0)
		return false;

	participants kernel("Jinda Jiri"), vacule("Vaculkova Leona"), krystal("Cukr Matej");
	vector<string> ind_stats_csv;
	ind_stats_csv.assign(lines.begin()+3, lines.end());
	string spolecno_v_hovoru = "spolecne";
	regex reg("(?:\\D+)(\\d+\\D+\\d+\\D+\\d+)(?:\.+)");
	smatch sm;
	regex_match(lines[1], sm, reg);

	individual_stats(ref(kernel), ref(vacule), ref(krystal), ind_stats_csv);
	alone_and_together_call_time(ref(kernel), ref(vacule), ref(krystal), ind_stats_csv,
		ref(spolecno_v_hovoru), sm[1].str(), format_by);

	int ucastnici[] = { kernel.ucastnik, vacule.ucastnik, krystal.ucastnik };
	string v_hovoru[] = { spolecno_v_hovoru, kernel.v_hovoru, vacule.v_hovoru, krystal.v_hovoru };
	string pocet_odpojeni[] = { kernel.pocet_odpojeni_str, vacule.pocet_odpojeni_str, krystal.pocet_odpojeni_str };
	string sam_v_hovoru[] = { kernel.sam_v_hovoru, vacule.sam_v_hovoru, krystal.sam_v_hovoru };

	call hovor(datum_hovoru, ucastnici, delka_hovoru, v_hovoru, pocet_odpojeni, sam_v_hovoru, format_by);
	all_calls.insert(hovor);

	string new_name = make_new_file_name(datum_hovoru, delka_hovoru);
	if (new_name.size() <= 1)
		cout << "Making new file name failed." << endl;
	else
		new_file_name.append(new_name);

	return true;
}



int main(void) {

	multiset<call, mensi> all_calls;

	string out_file = "reports\\_result.txt";

	int number_of_all_calls = 0, number_of_errors = 0;

	int i = 1;
	while (true) {
		string in_file = "reports\\";
		string new_file_name(in_file);

		string elem = "1 (";
		elem.append(to_string(i));
		elem.append(").csv");

		in_file.append(elem);

		// overeni, zda dany soubor existuje, pokud ne tak se cyklus ukonci
		if (FILE* file = fopen(in_file.c_str(), "r"))
			fclose(file);
		else
			break;

		if (!process_file(in_file, out_file, ref(all_calls), ref(new_file_name))) {
			number_of_errors++;
			cout << "Problem se souborem: " << in_file << endl;
		}
		if (rename(in_file.c_str(), new_file_name.c_str()) != 0)
			perror("Error renaming file");
		number_of_all_calls++;
		i++;
	}

	//for (auto i : all_calls)
	//	i.print();

	if (!write_to_file(out_file, all_calls)) {
		cout << "Error occured while writing to file." << endl;
		return false;
	}

	cout << endl;
	cout << number_of_all_calls - number_of_errors << " z " << number_of_all_calls;
	cout << " zaznamu bylo uspesne zpracovano." << endl;

}