export type LogLevel = "INFO" | "WARN" | "ERR";

export type LogEntry = {
	timestamp: string;
	type: LogLevel;
	message: string;
};
