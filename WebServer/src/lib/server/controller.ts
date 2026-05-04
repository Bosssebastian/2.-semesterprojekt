import { env } from '$env/dynamic/private';
import type { LogEntry } from '$lib/logs';

const DEFAULT_CONTROLLER_BASE_URL = 'http://127.0.0.1:8081';

export type ControllerStatus = {
	online: boolean;
	state: string | null;
	error: string | null;
};

export type CurrentSample = {
	t: number;
	a: number;
};

function getControllerBaseUrl() {
	return env.CONTROLLER_BASE_URL ?? DEFAULT_CONTROLLER_BASE_URL;
}

async function parseJsonResponse(response: Response) {
	const contentType = response.headers.get('content-type') ?? '';
	if (!contentType.includes('application/json')) {
		return null;
	}

	try {
		return await response.json();
	} catch {
		return null;
	}
}

export async function fetchControllerState(fetchImpl: typeof fetch) {
	const response = await fetchImpl(`${getControllerBaseUrl()}/getstate`);
	const payload = await parseJsonResponse(response);

	if (!response.ok) {
		throw new Error(
			typeof payload?.error === 'string' ? payload.error : 'Failed to fetch controller state'
		);
	}

	const uiState =
		typeof payload?.stateLabel === 'string' && payload.stateLabel.length > 0
			? payload.stateLabel
			: payload?.state;

	if (typeof uiState !== 'string' || uiState.length === 0) {
		throw new Error('Controller state response did not include a valid state');
	}

	return uiState;
}

export async function fetchControllerStatus(fetchImpl: typeof fetch): Promise<ControllerStatus> {
	try {
		return {
			online: true,
			state: await fetchControllerState(fetchImpl),
			error: null
		};
	} catch (error) {
		return {
			online: false,
			state: null,
			error: error instanceof Error ? error.message : 'Failed to reach controller'
		};
	}
}

function isLogEntry(value: unknown): value is LogEntry {
	if (typeof value !== 'object' || value === null) {
		return false;
	}

	const entry = value as Record<string, unknown>;

	return (
		typeof entry.timestamp === 'string' &&
		typeof entry.message === 'string' &&
		(entry.type === 'INFO' || entry.type === 'WARN' || entry.type === 'ERR')
	);
}

export async function fetchControllerLogs(fetchImpl: typeof fetch): Promise<LogEntry[]> {
	const response = await fetchImpl(`${getControllerBaseUrl()}/logs`);
	const payload = await parseJsonResponse(response);

	if (!response.ok) {
		throw new Error(
			typeof payload?.error === 'string' ? payload.error : 'Failed to fetch controller logs'
		);
	}

	if (!Array.isArray(payload?.entries) || !payload.entries.every(isLogEntry)) {
		throw new Error('Controller log response did not include valid log entries');
	}

	return payload.entries;
}

function isCurrentSample(value: unknown): value is CurrentSample {
	if (typeof value !== 'object' || value === null) {
		return false;
	}

	const sample = value as Record<string, unknown>;
	return typeof sample.t === 'number' && typeof sample.a === 'number';
}

export async function fetchGripperCurrent(
	fetchImpl: typeof fetch,
	windowMs = 180000
): Promise<CurrentSample[]> {
	const response = await fetchImpl(`${getControllerBaseUrl()}/gripper/current?windowMs=${windowMs}`);
	const payload = await parseJsonResponse(response);

	if (!response.ok) {
		throw new Error(
			typeof payload?.error === 'string' ? payload.error : 'Failed to fetch gripper current'
		);
	}

	if (!Array.isArray(payload?.samples) || !payload.samples.every(isCurrentSample)) {
		throw new Error('Gripper current response did not include valid samples');
	}

	return payload.samples;
}

export async function sendControllerCommand(
	fetchImpl: typeof fetch,
	command: 'start' | 'stop' | 'skip' | 'reset'
) {
	try {
		const path =
			command === 'start'
				? '/cmdStart'
				: command === 'stop'
					? '/cmdStop'
					: command === 'skip'
						? '/cmdSkipReq'
						: '/cmdReset';
		const response = await fetchImpl(`${getControllerBaseUrl()}${path}`, {
			method: 'POST'
		});

		if (!response.ok) {
			return {
				ok: false,
				status: response.status,
				error: `Failed to send ${command} command`,
				offline: response.status >= 500
			};
		}

		return {
			ok: true,
			status: response.status,
			error: null as string | null,
			offline: false
		};
	} catch (error) {
		return {
			ok: false,
			status: 503,
			error:
				error instanceof Error ? error.message : `Failed to send ${command} command`,
			offline: true
		};
	}
}
