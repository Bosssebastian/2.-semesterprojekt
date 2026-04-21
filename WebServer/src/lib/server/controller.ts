import { env } from '$env/dynamic/private';

const DEFAULT_CONTROLLER_BASE_URL = 'http://127.0.0.1:8081';

export type ControllerStatus = {
	online: boolean;
	state: string | null;
	error: string | null;
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

	if (typeof payload?.state !== 'string' || payload.state.length === 0) {
		throw new Error('Controller state response did not include a valid state');
	}

	return payload.state;
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

export async function sendControllerCommand(
	fetchImpl: typeof fetch,
	command: 'start' | 'stop'
) {
	try {
		const path = command === 'start' ? '/cmdStart' : '/cmdStop';
		const response = await fetchImpl(`${getControllerBaseUrl()}${path}`, {
			method: 'POST'
		});
		const payload = await parseJsonResponse(response);

		if (!response.ok) {
			const errorMessage =
				typeof payload?.error === 'string'
					? payload.error
					: `Failed to send ${command} command`;
			return {
				ok: false,
				status: response.status,
				error: errorMessage,
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
